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
from optparse import OptionParser
import os
import os.path
import posixpath
from select import select
import SimpleHTTPServer
from subprocess import Popen
import sys
import thread
import urllib

class TestProcess(object):
  def __init__(self, runner, args):
    self._runner = runner
    self._rfd = -1

    (self._rfd, wfd) = os.pipe()
    thread.start_new_thread(self._run_child, (args, wfd))

  def _run_child(self, args, wfd):
    w = os.fdopen(wfd, "w")
    p = Popen(args)

    w.write(str(p.wait()))

  def handle_event(self):
    self.returncode = int(os.fdopen(self._rfd, "r").read())
    self._rfd = -1
    self._runner.remove_source(self)
    self._runner.quit()

  def fileno(self):
    return self._rfd

class TestHTTPRequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
  def translate_path(self, path):
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

class Runner(object):
  def __init__(self, options):
    self._should_shutdown = False
    self._rlist = []
    self._p = None

    (opts, args) = options.parse_args()

    http_path = os.path.abspath(opts.server) if opts.server is not None else None
    http_port = opts.port if opts.port is not None else 8080

    if http_path is not None:
      server = TestHTTPServer(("", http_port), TestHTTPRequestHandler, http_path)
      self.add_source(server)

    if len(args) > 0:
      self._p = TestProcess(self, args)
      self.add_source(self._p)

  def add_source(self, source):
    if not hasattr(source, "handle_event"):
      raise Exception("Invalid source")

    self._rlist.append(source)

  def remove_source(self, source):
    if not source in self._rlist:
      raise Exception("Source doesn't exist")

    self._rlist.remove(source)

  def run(self):
    while not self._should_shutdown:
      (r, w, x) = select(self._rlist, [], [])
      for i in r:
        i.handle_event()

    return self._p.returncode if self._p is not None else 0

  def quit(self):
    self._should_shutdown = True

def main():
  parser = Options()
  return Runner(parser).run()
    
if __name__ == "__main__":
  sys.exit(main())
