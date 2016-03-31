#!/usr/bin/python
# vim:expandtab:shiftwidth=2:tabstop=2:

# Copyright (C) 2013-2016 Canonical Ltd.

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

from __future__ import print_function

from optparse import OptionParser
import os
import os.path
from subprocess import Popen
import sys
import thread
import yaml

sys.dont_write_bytecode = True
os.environ["PYTHONDONTWRITEBYTECODE"] = "1"

sys.path.insert(0, os.path.join(os.path.dirname(__file__), os.pardir, os.pardir, "build", "python"))
from eventloop import EventLoop
from httpserver import TestHTTPServer, TestHTTPRequestHandler
from utils import ScopedTmpdir, TOPSRCDIR

SERVER_CONFIGS = [
  { "port": 8080 },
  { "port": 4443, "sslcert": "qt/tests/ssldata/OxideTest" },
  { "port": 4444, "sslcert": "qt/tests/ssldata/OxideTestExpired" },
  { "port": 4445, "sslcert": "qt/tests/ssldata/OxideTestSelfSigned" },
  { "port": 4446, "sslcert": "qt/tests/ssldata/OxideTestBadIdentity" }
]

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

def load_config(filename):
  with open(filename, "r") as fd:
    return yaml.load(fd.read());

def get_debugger_args(options):
  if not options.debug:
    return None

  return ["gdb", "--args"]

class Options(OptionParser):
  def __init__(self):
    OptionParser.__init__(self)

    self.add_option("--config", dest="config",
                    help="Path to the test configuration file")
    self.add_option("--debug", dest="debug", action="store_true",
                    help="Run the test in a debugger (only supported is GDB)")

class Runner(object):
  def __init__(self):
    self._event_loop = EventLoop()
    self._p = None

  def run(self, options):
    (opts, args) = options.parse_args()

    if not opts.config:
      print("Missing --config option", file=sys.stderr)
      sys.exit(1)

    config = load_config(opts.config)

    debugger_args = get_debugger_args(opts)

    with ScopedTmpdir(prefix="tmp-oxide-runtests") as tmpdir:
      return self._run_with_tmpdir(tmpdir, config, debugger_args, args)

  def _run_with_tmpdir(self, tmpdir, config, debugger_args, extra_args):
    os.environ["OXIDE_TESTING_MODE"] = "1"

    for server in SERVER_CONFIGS:
      server = TestHTTPServer(server["port"],
                              config["http_server_dir"],
                              os.path.join(TOPSRCDIR, server["sslcert"]) if "sslcert" in server else None)
      self._event_loop.add_reader(server, server.handle_event)

    test_args = []
    if debugger_args:
      test_args.extend(debugger_args)

    test_args.extend(
        [ config["exec"],
          "--qml-import-path", config["qml_import_path"],
          "--qt-plugin-path", config["qt_plugin_path"],
          "--nss-db-path", os.path.join(TOPSRCDIR, "qt/tests/ssldata/nss"),
          "--tmpdir", tmpdir ])

    test_args.extend(extra_args)

    self._p = TestProcess(test_args)
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
