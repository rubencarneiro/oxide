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

import os
import os.path
from subprocess import check_call, Popen, CalledProcessError

DEPOT_TOOLS = "https://chromium.googlesource.com/chromium/tools/depot_tools.git"
DEPOT_TOOLS_REV = "3b5efdf64d35ba20f8afd4f568eb190c3f1e8e12"

topsrcdir = os.path.abspath(os.path.join(__file__, os.pardir))
chromiumdir = os.path.join(topsrcdir, "chromium")

def CheckCall(args, cwd = None):
  if cwd is None:
    check_call(args)
  else:
    r = Popen(args, cwd=cwd).wait()
    if r is not 0: raise CalledProcessError(r, args)

def main():
  depot_tools_path = os.path.join(chromiumdir, "depot_tools")
  if not os.path.isdir(depot_tools_path):
    check_call(["git", "clone", DEPOT_TOOLS, depot_tools_path])

  CheckCall(["git", "config", "remote.origin.url", DEPOT_TOOLS], depot_tools_path)
  CheckCall(["git", "pull", "origin", "master"], depot_tools_path)
  CheckCall(["git", "checkout", DEPOT_TOOLS_REV], depot_tools_path)
  os.environ["PATH"] = os.path.join(chromiumdir, "depot_tools") + ":" + os.getenv("PATH")

  chromium_src_path = os.path.join(chromiumdir, "src")
  has_hg = os.path.isdir(os.path.join(chromium_src_path, ".hg"))
  if has_hg:
    CheckCall(["hg", "qpop", "-a"], chromium_src_path)
    os.rename(os.path.join(chromium_src_path, ".hg"),
              os.path.join(chromium_src_path, ".hg.bak"))

  CheckCall(["gclient", "sync", "--force",
             "--gclientfile", os.path.join(topsrcdir, "gclient.conf")],
            chromiumdir)

  if not has_hg:
    with open(os.path.join(chromium_src_path, ".hgignore"), "w") as f:
      f.write("~$\n")
      f.write("\.svn/\n")
      f.write("\.git/\n")
      f.write("^out/\n")
      f.write("\.host\.(.*\.|)mk$\n")
      f.write("\.target\.(.*\.|)mk$\n")
      f.write("Makefile(\.*|)$\n")
      f.write("^\.hgignore$\n")
      f.write("\.pyc$\n")
    CheckCall(["hg", "init"], chromium_src_path)
    CheckCall(["hg", "addremove"], chromium_src_path)
    CheckCall(["hg", "ci", "-m", "Updated with client.py"], chromium_src_path)
    CheckCall(["hg", "qinit"], chromium_src_path)

    patchdir = os.path.join(topsrcdir, "patches")
    patches = []
    with open(os.path.join(patchdir, "series"), "r") as f:
      for line in f.readlines():
        patches.append(os.path.join(patchdir, line.strip()))
    patches.reverse()
    for patch in patches:
      CheckCall(["hg", "qimport", patch], chromium_src_path)
  else:
    os.rename(os.path.join(chromium_src_path, ".hg.bak"),
              os.path.join(chromium_src_path, ".hg"))
    CheckCall(["hg", "addremove"], chromium_src_path)
    try:
      CheckCall(["hg", "ci", "-m", "Updated with client.py"], chromium_src_path)
    except CalledProcessError as error:
      if error.returncode != 1:
        raise

  CheckCall(["hg", "qpush", "-a"], chromium_src_path)

if __name__ == "__main__":
  main()
