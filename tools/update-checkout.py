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
from ConfigParser import ConfigParser, NoOptionError
from optparse import OptionParser
import os
import os.path
import sys

sys.dont_write_bytecode = True
os.environ["PYTHONDONTWRITEBYTECODE"] = "1"

sys.path.insert(0, os.path.join(os.path.dirname(__file__), os.pardir, "build", "python"))
from constants import (
  OXIDE_DEPSFILE,
  TOP_SRCDIR
)
from utils import (
  CheckCall,
  LoadJsonFromPath
)

GCLIENT_SPEC_TEMPLATE = (
  "solutions = ["
    "{ \"name\": \"src\", "
      "\"url\": \"%(url)s\", "
      "\"deps_file\": \"DEPS\", "
      "\"managed\": True, "
      "\"custom_deps\": "
        "{"
          "\"build/scripts/command_wrapper/bin\": None, "
          "\"build/scripts/private/data/reliability\": None, "
          "\"build/scripts/tools/deps2git\": None, "
          "\"build/scripts/gsd_generate_index\": None, "
          "\"build/scripts/private/data/reliability\": None, "
          "\"build/third_party/cbuildbot_chromite\": None, "
          "\"build/third_party/lighttpd\": None, "
          "\"build/third_party/xvfb\": None, "
          "\"depot_tools\": None, "
          "\"src/chrome/tools/test/reference_build/chrome_win\": None, "
          "\"src/chrome/tools/test/reference_build/chrome_linux\": None, "
          "\"src/chrome/tools/test/reference_build/chrome_mac\": None, "
          "\"src/third_party/freetype-android/src\": None, "
          "\"src/third_party/hunspell_dictionaries\": None, "
          "%(custom_deps)s"
        "}, "
      "\"custom_hooks\": "
        "["
          "{ \"name\": \"check_git_config\", \"action\": [\"true\"] },"
          "{ \"name\": \"gyp\", \"action\": [\"true\"] },"
        "],"
      "\"safesync_url\": \"\", "
    "} "
  "]"
)

class Options(OptionParser):
  def __init__(self):
    OptionParser.__init__(self)

    self.add_option("-f", "--force", dest="force", action="store_true",
                    help="Force an update")

class Config(ConfigParser):
  def __init__(self):
    ConfigParser.__init__(self, allow_no_value=True)

    filename = os.path.join(TOP_SRCDIR, os.pardir, ".checkout.cfg")
    if not os.path.isfile(filename):
      print("Cannot find .checkout.cfg. Is this a checkout created with "
            "fetch_oxide.py?", file=sys.stderr)
      sys.exit(1)

    with open(filename, "r") as f:
      self.readfp(f)

  @property
  def cachedir(self):
    try:
      return self.get("DEFAULT", "cachedir")
    except NoOptionError:
      return None

def GetGclientSpec(cachedir):
  deps = LoadJsonFromPath(OXIDE_DEPSFILE)
  custom_deps = ""
  chromium_url = None
  for dep in deps:
    if dep == "src":
      chromium_url = "%s@%s" % (deps[dep]["origin"], deps[dep]["rev"])
      continue
    custom_deps += "\"%s\": \"%s@%s\", " % (dep,
                                            deps[dep]["origin"],
                                            deps[dep]["rev"])
  if not chromium_url:
    print("DEPS.oxide must have a src entry pointing to the Chromium GIT "
          "repository", file=sys.stderr)
    sys.exit(1)
  spec = GCLIENT_SPEC_TEMPLATE % { "url": chromium_url,
                                   "custom_deps": custom_deps }
  if cachedir:
    spec = "%s\ncache_dir = \"%s\"" % (spec, cachedir)
  return spec

def UpdateGclientConfig(config):
  # We don't use gclient config here, because it doesn't support both
  # --spec and --cache-dir, and --spec doesn't support newlines
  with open(os.path.join(TOP_SRCDIR, os.pardir, ".gclient"), "w") as fd:
    fd.write(GetGclientSpec(config.cachedir))

def SyncCheckout(force):
  args = ["gclient", "sync", "-D", "--with_branch_heads"]
  if force:
    args.append("--force")
  CheckCall(args, os.path.join(TOP_SRCDIR, os.pardir))

def main():
  o = Options()
  (options, args) = o.parse_args()

  c = Config()
  UpdateGclientConfig(c)
  SyncCheckout(options.force)

if __name__ == "__main__":
  main()
