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
from urlparse import urlsplit

sys.dont_write_bytecode = True
os.environ["PYTHONDONTWRITEBYTECODE"] = "1"

sys.path.insert(0, os.path.join(os.path.dirname(__file__), os.pardir, "build", "python"))
from constants import (
  OXIDEDEPS_FILE,
  OXIDESRC_DIR,
  TOP_DIR,
  TOPSRC_DIRNAME
)
from utils import (
  CheckCall,
  CheckOutput,
  GetGitConfig,
  LoadJsonFromPath
)

GCLIENT_SPEC_TEMPLATE = (
  "solutions = ["
    "{ \"name\": \"%(name)s\", "
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
          "\"%(name)s/chrome/tools/test/reference_build/chrome_win\": None, "
          "\"%(name)s/chrome/tools/test/reference_build/chrome_linux\": None, "
          "\"%(name)s/chrome/tools/test/reference_build/chrome_mac\": None, "
          "\"%(name)s/third_party/freetype-android/src\": None, "
          "\"%(name)s/third_party/hunspell_dictionaries\": None, "
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

def RewriteOrigin(origin, user_id):
  if not user_id:
    return origin
  u = urlsplit(origin)
  if u.scheme not in ("git", "https"):
    return origin
  if u.netloc != "git.launchpad.net":
    return origin
  u = u._replace(scheme="ssh")
  u = u._replace(netloc="%s@%s" % (user_id, u.netloc))
  return u.geturl()

def GetGclientSpec(cache_dir, cache_mode, user_id):
  deps = LoadJsonFromPath(OXIDEDEPS_FILE)
  custom_deps = ""
  chromium_url = None
  for dep in deps:
    if dep == TOPSRC_DIRNAME:
      chromium_url = "%s@%s" % (RewriteOrigin(deps[dep]["origin"], user_id),
                                deps[dep]["rev"])
      continue
    custom_deps += "\"%s\": \"%s@%s\", " % (dep,
                                            RewriteOrigin(deps[dep]["origin"], user_id),
                                            deps[dep]["rev"])
  if not chromium_url:
    print("DEPS.oxide must have a src entry pointing to the Chromium GIT "
          "repository", file=sys.stderr)
    sys.exit(1)
  spec = GCLIENT_SPEC_TEMPLATE % { "url": chromium_url,
                                   "name": TOPSRC_DIRNAME,
                                   "custom_deps": custom_deps }
  if cache_dir:
    spec = "%s\ncache_dir = \"%s\"" % (spec, cache_dir)
    if cache_mode:
      spec = "%s\ncache_mode = \"%s\"" % (spec, cache_mode)
  return spec

def UpdateGclientConfig(cache_dir, cache_mode, user_id):
  # We don't use gclient config here, because it doesn't support both
  # --spec and --cache-dir, and --spec doesn't support newlines
  with open(os.path.join(TOP_DIR, ".gclient"), "w") as fd:
    fd.write(GetGclientSpec(cache_dir, cache_mode, user_id))

def SyncCheckout(force):
  args = ["gclient", "sync", "-D", "--with_branch_heads"]
  if force:
    args.append("--force")
  CheckCall(args, TOP_DIR)

def main():
  o = Options()
  (options, args) = o.parse_args()

  cache_dir = GetGitConfig("oxide.cacheDir", OXIDESRC_DIR)
  cache_mode = GetGitConfig("oxide.cacheMode", OXIDESRC_DIR)
  user_id = GetGitConfig("oxide.launchpadUserId", OXIDESRC_DIR)
  UpdateGclientConfig(cache_dir, cache_mode, user_id)
  SyncCheckout(options.force)

if __name__ == "__main__":
  main()
