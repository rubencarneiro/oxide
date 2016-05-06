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
import sys

sys.dont_write_bytecode = True
os.environ["PYTHONDONTWRITEBYTECODE"] = "1"

sys.path.insert(0, os.path.join(os.path.dirname(__file__), os.pardir, "build", "python"))
from constants import (
  OXIDEDEPS_FILE,
  TOPSRC_DIR
)
from utils import (
  CheckCall,
  LoadJsonFromPath
)

class Options(OptionParser):
  def __init__(self):
    OptionParser.__init__(self, description="""
Add upstream remotes to all repositories listed in DEPS.oxide with an
upstream URL
""")

    self.add_option("--no-fetch", action="store_true",
                    help="Don't run 'git fetch upstream' after adding the "
                         "upstream remotes")

def main():
  (options, args) = Options().parse_args()

  topsrc_dir_parent = os.path.dirname(TOPSRC_DIR)

  deps = LoadJsonFromPath(OXIDEDEPS_FILE)
  for dep in deps:
    if "upstream" not in deps[dep]:
      continue
    path = os.path.join(topsrc_dir_parent, dep)
    try:
      CheckCall(["git", "remote", "show", "upstream"], path, True)
      print("Repository '%s' already has a remote called 'upstream'" % path)
    except:
      print("Adding 'upstream' remote to repository '%s'" % path)
      CheckCall(["git", "remote", "add", "upstream",
                 deps[dep]["upstream"]], path)
      extra_refs = [ "refs/branch-heads/*" ]
      if dep == "src":
        extra_refs.append("refs/tags/*")
      for r in extra_refs:
        CheckCall(["git", "config", "--add", "remote.upstream.fetch",
                   "+%s:%s" % (r, r)], path)
      if not options.no_fetch:
        CheckCall(["git", "fetch", "upstream"], path)

if __name__ == "__main__":
  main()
