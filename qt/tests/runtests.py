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

from optparse import OptionParser
import os
import os.path
from subprocess import Popen
import sys
import thread

sys.dont_write_bytecode = True
os.environ["PYTHONDONTWRITEBYTECODE"] = "1"

sys.path.insert(0, os.path.join(os.path.dirname(__file__), os.pardir, os.pardir, "build", "python"))
from eventloop import EventLoop
from httpserver import TestHTTPServer, TestHTTPRequestHandler
from oxide_utils import ScopedTmpdir

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

class Options(OptionParser):
  def __init__(self):
    OptionParser.__init__(self)

    self.add_option("--server-dir", dest="server_dir",
                    help="Run a HTTP server from the specified directory", default=os.getcwd())
    self.add_option("--server-config", action="append", dest="server_configs",
                    help="Specify a HTTP server config (port:certfile)", default=[])

class Runner(object):
  def __init__(self):
    self._event_loop = EventLoop()

    self._p = None

  def run(self, options):
    with ScopedTmpdir(prefix="tmp-oxide-runtests") as tmpdir:
      os.environ["OXIDE_RUNTESTS_TMPDIR"] = tmpdir

      (opts, args) = options.parse_args()

      for c in opts.server_configs:
        config = c.split(":")
        server = TestHTTPServer(config[0], opts.server_dir, config[1] if len(config) > 1 else None)
        self._event_loop.add_reader(server, server.handle_event)

      if len(args) > 0:
        self._p = TestProcess(args)
        self._event_loop.add_reader(self._p, self._p.handle_event, self._event_loop)

      self._event_loop.run()
      self._event_loop.close()
      assert self._p.returncode != None
      return self._p.returncode

def main():
  parser = Options()
  return Runner().run(parser)
    
if __name__ == "__main__":
  sys.exit(main())
