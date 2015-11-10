#!/usr/bin/python
# vim:expandtab:shiftwidth=2:tabstop=2:

# Copyright (C) 2013-2015 Canonical Ltd.

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
import re
import shutil
import sys
from urlparse import urljoin, urlsplit

sys.dont_write_bytecode = True
os.environ["PYTHONDONTWRITEBYTECODE"] = "1"

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "build", "python"))
from oxide_utils import CheckCall, CheckOutput, GetChecksum, GetFileChecksum, CHROMIUMSRCDIR, TOPSRCDIR

DEPOT_TOOLS_GIT_URL = "https://chromium.googlesource.com/chromium/tools/depot_tools.git"
DEPOT_TOOLS_GIT_REV = "c1ae89ecd635abfea2d94e5b49c7d92f49f28f22"
DEPOT_TOOLS_PATH = os.path.join(TOPSRCDIR, "third_party", "depot_tools")
DEPOT_TOOLS_OLD_PATH = os.path.join(TOPSRCDIR, "chromium", "depot_tools")

CHROMIUM_GIT_URL = "https://git.launchpad.net/~oxide-developers/oxide/+git/chromium"
CHROMIUM_GCLIENT_SPEC = (
  "solutions = ["
    "{ \"name\": \"src\", "
      "\"url\": \"%s\", "
      "\"deps_file\": \".DEPS.git\", "
      "\"managed\": False, "
      "\"custom_deps\": "
        "{ \"build/scripts/command_wrapper/bin\": None, "
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
          "\"src/third_party/hunspell_dictionaries\": None, "
        "}, "
      "\"safesync_url\": \"\", "
    "} "
  "]"
)
CHROMIUM_GCLIENT_FILE = os.path.join(os.path.dirname(CHROMIUMSRCDIR), ".gclient")

def IsGitRepo(path):
  try:
    CheckCall(["git", "status"], path, quiet=True)
  except:
    return False
  return True

def GitRepoHeadMatchesId(path, id):
  head = CheckOutput(["git", "rev-parse", "HEAD"], path).strip()
  if head == id:
    return True
  for tag in CheckOutput(["git", "tag", "--contains", head], path).splitlines():
    if tag.strip() == id:
      return True
  return False

def GetMirrorPath(cachedir, url):
  try:
    return CheckOutput([sys.executable, os.path.join(DEPOT_TOOLS_PATH, "git_cache.py"),
                        "exists", "--cache-dir", cachedir, url]).strip()
  except subprocess.CalledProcessError:
    return None

def PopulateGitMirror(cachedir, url, refs = []):
  args = [sys.executable, os.path.join(DEPOT_TOOLS_PATH, "git_cache.py"),
          "populate", "--cache-dir", cachedir]
  for r in refs:
    args.extend(["--ref", r])
  args.append(url)
  CheckCall(args)

def InitGitRepo(url, path, cachedir = None, additional_refs = []):
  if IsGitRepo(path):
    return

  if os.path.isdir(path):
    shutil.rmtree(path)
  elif os.path.isfile(path):
    os.remove(path)

  args = ["git", "clone"]

  if cachedir:
    PopulateGitMirror(cachedir, url, additional_refs)
    url = GetMirrorPath(cachedir, url)
    args.append("--shared")

  args.extend([url, path])
  CheckCall(args)

  for r in additional_refs:
    CheckCall(["git", "config", "--add", "remote.origin.fetch",
               "+%s:%s" % (r, r)], CHROMIUMSRCDIR)

def UpdateGitRepo(url, path, commit, cachedir = None):
  newly_cloned = False
  if not IsGitRepo(path):
    InitGitRepo(url, path, cachedir)
    newly_cloned = True

  if cachedir:
    if not newly_cloned:
      PopulateGitMirror(cachedir, url)
    url = GetMirrorPath(cachedir, url)

  current_url = CheckOutput(["git", "config", "remote.origin.url"], path).strip()
  if current_url != url:
    CheckCall(["git", "remote", "set-url", "origin", url], path)

  CheckCall(["git", "fetch"], path)
  CheckCall(["git", "checkout", commit], path)

def GetDesiredChromiumVersion():
  with open(os.path.join(TOPSRCDIR, "CHROMIUM_VERSION"), "r") as fd:
    return fd.read().strip()

def GetChromiumGclientSpec(cachedir):
  spec = CHROMIUM_GCLIENT_SPEC % CHROMIUM_GIT_URL
  if cachedir:
    spec = "%s\ncache_dir = \"%s\"" % (spec, cachedir)
  return spec

def PrepareDepotTools():
  if IsGitRepo(DEPOT_TOOLS_OLD_PATH) and not IsGitRepo(DEPOT_TOOLS_PATH):
    os.rename(DEPOT_TOOLS_OLD_PATH, DEPOT_TOOLS_PATH)

  UpdateGitRepo(DEPOT_TOOLS_GIT_URL, DEPOT_TOOLS_PATH, DEPOT_TOOLS_GIT_REV)

  sys.path.insert(0, DEPOT_TOOLS_PATH)
  os.environ["PATH"] = DEPOT_TOOLS_PATH + ":" + os.getenv("PATH")

def NeedsChromiumSync(config):
  # Check that CHROMIUMSRCDIR is a git repo
  if not IsGitRepo(CHROMIUMSRCDIR):
    return True

  if not GitRepoHeadMatchesId(CHROMIUMSRCDIR,
                              GetDesiredChromiumVersion()):
    return True

  # Sync if there is no .gclient file
  if not os.path.isfile(CHROMIUM_GCLIENT_FILE):
    return True

  # Sync if the gclient spec has changed
  if (GetFileChecksum(CHROMIUM_GCLIENT_FILE) !=
      GetChecksum(GetChromiumGclientSpec(config.cachedir))):
    return True

  return False

def SyncChromium(config):
  if os.path.isdir(os.path.join(CHROMIUMSRCDIR, ".hg")):
    shutil.rmtree(os.path.join(CHROMIUMSRCDIR, ".hg"))
    os.remove(os.path.join(CHROMIUMSRCDIR, ".hgignore"))

  chromium_dir = os.path.dirname(CHROMIUMSRCDIR)
  if not os.path.isdir(chromium_dir):
    os.makedirs(chromium_dir)

  # We don't use gclient.py config here, because it doesn't support both
  # --spec and --cache-dir, and --spec doesn't support newlines
  with open(os.path.join(chromium_dir, ".gclient"), "w") as f:
    f.write(GetChromiumGclientSpec(config.cachedir))

  refs = [ "refs/tags/*" ]

  if not IsGitRepo(CHROMIUMSRCDIR):
    InitGitRepo(CHROMIUM_GIT_URL, CHROMIUMSRCDIR, config.cachedir, refs)
    CheckCall(["git", "submodule", "foreach",
               "git config -f $toplevel/.git/config submodule.$name.ignore all"],
              CHROMIUMSRCDIR)
    CheckCall(["git", "config", "diff.ignoreSubmodules", "all"],
              CHROMIUMSRCDIR)
  UpdateGitRepo(CHROMIUM_GIT_URL, CHROMIUMSRCDIR,
                GetDesiredChromiumVersion(), config.cachedir)

  CheckCall([sys.executable, os.path.join(DEPOT_TOOLS_PATH, "gclient.py"),
             "sync", "-D", "--with_branch_heads"], chromium_dir)

class Options(OptionParser):
  def __init__(self):
    OptionParser.__init__(self)

    self.add_option("-c", "--cache-dir", dest="cache_dir",
                    help="Specify a directory for a local mirror")

class Config(ConfigParser):
  def __init__(self, opts):
    ConfigParser.__init__(self, allow_no_value=True)

    filename = os.path.join(TOPSRCDIR, ".client.cfg")
    try:
      with open(filename, "r") as f:
        self.readfp(f)
    except IOError as e:
      if e.errno != 2:
        raise
      if opts.cache_dir:
        self.set("DEFAULT", "cachedir", opts.cache_dir)

    if opts.cache_dir and opts.cache_dir != self.cachedir:
      print("Ignoring value passed to --cache-dir, as it doesn't match the "
            "value passed when the checkout was created", file=sys.stderr)

    with open(filename, "w") as f:
      self.write(f)

  @property
  def cachedir(self):
    try:
      return self.get("DEFAULT", "cachedir")
    except NoOptionError:
      return None

def main():
  o = Options()
  (options, args) = o.parse_args()
  if options.cache_dir:
    options.cache_dir = os.path.abspath(options.cache_dir)

  c = Config(options)

  PrepareDepotTools()

  old_chromium_dir = os.path.join(TOPSRCDIR, "chromium")
  if os.path.isdir(old_chromium_dir):
    shutil.rmtree(old_chromium_dir)
  release_deps_dir = os.path.join(os.path.dirname(CHROMIUMSRCDIR), "release_deps")
  if os.path.isdir(release_deps_dir):
    shutil.rmtree(release_deps_dir)

  if NeedsChromiumSync(c):
    SyncChromium(c)

if __name__ == "__main__":
  main()
