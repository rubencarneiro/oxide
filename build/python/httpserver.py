#!/usr/bin/python
# vim:expandtab:shiftwidth=2:tabstop=2:

# Copyright (C) 2014 Canonical Ltd.

# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

import BaseHTTPServer
from cStringIO import StringIO
import os
import os.path
import posixpath
import shutil
import SimpleHTTPServer
import ssl
import traceback
import urllib

class PythonHandlerSandboxGlobal(dict):
  def __init__(self):
    dict.__init__(self, {
      '__builtins__': __builtins__
    })

class PythonHandlerSandbox(object):
  def __init__(self, f):
    self._globals = PythonHandlerSandboxGlobal()
    execfile(f, self._globals)

  def run_handler(self, request):
    self._globals["handler"](request)

class PythonHTTPRequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):
  def __init__(self, request, translated_path):
    self.command = request.command
    self.path = request.path
    self.request_version = request.request_version
    self.headers = request.headers
    self.rfile = request.rfile
    self.requestline = request.requestline

    self._request = request
    self._translated_path = translated_path

    BaseHTTPServer.BaseHTTPRequestHandler.__init__(
        self, None, request.client_address, request.server)

  def setup(self):
    self.wfile = StringIO()

  def handle(self):
    try:
      sandbox = PythonHandlerSandbox(self._translated_path)
    except OSError:
      self._request.send_error(404)
      return
    except:
      self._request.send_error(500)
      traceback.print_exc()
      return

    del self._translated_path

    request = self._request
    del self._request
    
    try:
      sandbox.run_handler(self)
    except:
      request.send_error(500)
      traceback.print_exc()

    self._request = request
    self._success = True

  def finish(self):
    try:
      self.finish_internal()
    except:
      traceback.print_exc()

  def finish_internal(self):
    if not hasattr(self, "_success") or not self._success:
      return

    if self.wfile.closed:
      self._request.send_error(500, "Python hook closed the write buffer")
      return

    self.wfile.flush()
    self.wfile.seek(0)
    shutil.copyfileobj(self.wfile, self._request.wfile)
    self.wfile.close()

class TestHTTPRequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
  def is_python_request(self):
    (root, ext) = os.path.splitext(self.translate_path(self.path))
    if ext == ".py":
      return True

    return False

  def execute_python(self):
    path = self.translate_path(self.path)
    if not os.path.isfile(path):
      return False

    PythonHTTPRequestHandler(self, path)

    return True

  def do_GET(self):
    """Serve a GET request."""
    if self.is_python_request() and self.execute_python():
      return

    f = self.send_head()
    if f:
      self.copyfile(f, self.wfile)
      f.close()

  def do_HEAD(self):
    """Serve a HEAD request."""
    if self.is_python_request() and self.execute_python():
      return

    f = self.send_head()
    if f:
      f.close()

  def translate_path(self, path):
    """Translate a /-separated PATH to the local filename syntax.

    Components that mean special things to the local file system
    (e.g. drive or directory names) are ignored.  (XXX They should
    probably be diagnosed.)

    """
    # abandon query parameters
    path = path.split('?',1)[0]
    path = path.split('#',1)[0]
    path = posixpath.normpath(urllib.unquote(path))
    words = path.split('/')
    words = filter(None, words)
    path = self.server.path
    for word in words:
      drive, word = os.path.splitdrive(word)
      head, word = os.path.split(word)
      if word in (os.curdir, os.pardir): continue
      path = os.path.join(path, word)
    (root, ext) = os.path.splitext(path)
    # rewrite html queries as python ones if the html file doesn't exist
    if ext == ".html" and not os.path.isfile(path):
      py_path = root + ".py"
      if os.path.isfile(py_path):
        return py_path
    return path

class TestHTTPServer(BaseHTTPServer.HTTPServer):
  def __init__(self, port, path, cert = None):
    BaseHTTPServer.HTTPServer.__init__(self, ("", int(port)), TestHTTPRequestHandler)
    self.path = path

    if cert:
      self.socket = ssl.wrap_socket(self.socket, keyfile=cert + ".key", certfile=cert + ".pem", server_side=True)

  def handle_event(self):
    self._handle_request_noblock()
