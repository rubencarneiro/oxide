#!/usr/bin/python
# vim:expandtab:shiftwidth=2:tabstop=2:

# Copyright (C) 2013 Canonical Ltd.

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
from optparse import OptionParser
import os
import os.path
import posixpath
import shutil
import SimpleHTTPServer
from subprocess import Popen
import sys
import tempfile
import thread
import traceback
import urllib

sys.dont_write_bytecode = True
os.environ["PYTHONDONTWRITEBYTECODE"] = "1"

sys.path.insert(0, os.path.join(os.path.dirname(__file__), os.pardir, os.pardir, "build", "python"))
from eventloop import EventLoop

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

class TestProcess(object):
  def __init__(self, args):
    self.returncode = None

    self._args = args
    (self._rfd, self._wfd) = os.pipe()

    thread.start_new_thread(self._run_child, ())

  def _run_child(self):
    w = os.fdopen(self._wfd, "w")
    p = Popen(self._args)

    w.write(str(p.wait()))

  def handle_event(self, event_loop):
    self.returncode = int(os.fdopen(self._rfd, "r").read())
    self._rfd = -1
    self._wfd = -1
    event_loop.quit()

  def fileno(self):
    return self._rfd

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
    return path

class TestHTTPServer(BaseHTTPServer.HTTPServer):
  def __init__(self, address, handler_class, path):
    BaseHTTPServer.HTTPServer.__init__(self, address, handler_class)
    self.path = path

  def handle_event(self):
    self._handle_request_noblock()

class Options(OptionParser):
  def __init__(self):
    OptionParser.__init__(self)

    self.add_option("-s", "--server", action="store", type="string", dest="server",
                    help="Run a HTTP server from the specified directory")
    self.add_option("-p", "--server-port", action="store", type="int", dest="port",
                    help="Specify a port for the HTTP server", default=8080)
    self.add_option("-t", "--temp-datadir", action="store_true", dest="temp_datadir",
                    help="Create a temporary data directory for oxide")

class Runner(object):
  def __init__(self, options):
    self._event_loop = EventLoop()

    self._temp_datadir = None
    self._http_server = None
    self._p = None

    (opts, args) = options.parse_args()

    if (opts.temp_datadir):
      self._temp_datadir = tempfile.mkdtemp()
      os.environ["OXIDE_TESTING_DATA_PATH"] = self._temp_datadir

    http_path = os.path.abspath(opts.server) if opts.server is not None else None
    http_port = opts.port if opts.port is not None else 8080

    if http_path is not None:
      self._http_server = TestHTTPServer(("", http_port), TestHTTPRequestHandler, http_path)
      self._event_loop.add_reader(self._http_server, self._http_server.handle_event)

    if len(args) > 0:
      self._p = TestProcess(args)
      self._event_loop.add_reader(self._p, self._p.handle_event, self._event_loop)

  def run(self):
    self._event_loop.run()
    self._event_loop.close()
    assert self._p.returncode != None
    return self._p.returncode

def main():
  parser = Options()
  return Runner(parser).run()
    
if __name__ == "__main__":
  sys.exit(main())
