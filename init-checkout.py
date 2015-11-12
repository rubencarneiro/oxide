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
import json
import os
import os.path
import re
import shutil
import sys
from urlparse import urlsplit

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

FORKED_GIT_REPOS_CONFIG = "FORKED_GIT_REPOS"

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

def AddExtraRefs(extra_refs, remote, path):
  for r in extra_refs:
    CheckCall(["git", "config", "--add", "remote.%s.fetch" % remote,
               "+%s:%s" % (r, r)], path)

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

  AddExtraRefs(additional_refs, "origin", CHROMIUMSRCDIR)

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

def RunMigration():
  old_chromium_dir = os.path.join(TOPSRCDIR, "chromium")
  if os.path.isdir(old_chromium_dir):
    shutil.rmtree(old_chromium_dir)
  release_deps_dir = os.path.join(os.path.dirname(CHROMIUMSRCDIR), "release_deps")
  if os.path.isdir(release_deps_dir):
    shutil.rmtree(release_deps_dir)
  hg_dir = os.path.join(CHROMIUMSRCDIR, ".hg")
  if os.path.isdir(hg_dir):
    os.rename(os.path.join(hg_dir, "patches"),
              os.path.join(CHROMIUMSRCDIR, "old_patches"))
    shutil.rmtree(hg_dir)

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

def SyncChromium(config, extra_refs, force):
  chromium_dir = os.path.dirname(CHROMIUMSRCDIR)
  if not os.path.isdir(chromium_dir):
    os.makedirs(chromium_dir)

  # We don't use gclient.py config here, because it doesn't support both
  # --spec and --cache-dir, and --spec doesn't support newlines
  with open(os.path.join(chromium_dir, ".gclient"), "w") as f:
    f.write(GetChromiumGclientSpec(config.cachedir))

  if not IsGitRepo(CHROMIUMSRCDIR):
    extra_refs = [ "refs/branch-heads/*" ]
    InitGitRepo(CHROMIUM_GIT_URL, CHROMIUMSRCDIR, config.cachedir, extra_refs)
    CheckCall(["git", "submodule", "foreach",
               "git config -f $toplevel/.git/config submodule.$name.ignore all"],
              CHROMIUMSRCDIR)
    CheckCall(["git", "config", "diff.ignoreSubmodules", "all"],
              CHROMIUMSRCDIR)
  UpdateGitRepo(CHROMIUM_GIT_URL, CHROMIUMSRCDIR,
                GetDesiredChromiumVersion(), config.cachedir)

  args = [sys.executable, os.path.join(DEPOT_TOOLS_PATH, "gclient.py"),
          "sync", "-D", "--with_branch_heads"]
  if force:
    args.append("--force")
  CheckCall(args, chromium_dir)

def AddUpstreamRemotes():
  config = {}
  with open(os.path.join(TOPSRCDIR, FORKED_GIT_REPOS_CONFIG), "r") as fd:
    config = json.load(fd)
  for p in config:
    path = os.path.join(TOPSRCDIR, p)
    try:
      CheckCall(["git", "remote", "show", "upstream"], path, True)
    except:
      CheckCall(["git", "remote", "add", "upstream", config[p]], path)
      extra_refs = [ "refs/branch-heads/*" ]
      if p == "third_party/chromium/src":
        extra_refs.append("refs/tags/*")
      AddExtraRefs(extra_refs, "upstream", path)
      CheckCall(["git", "fetch", "upstream"], path)

def AddGitPushUrls(userid):
  config = {}
  with open(os.path.join(TOPSRCDIR, FORKED_GIT_REPOS_CONFIG), "r") as fd:
    config = json.load(fd)
  for p in config:
    path = os.path.join(TOPSRCDIR, p)
    url = CheckOutput(["git", "config", "remote.origin.url"], path).strip()
    if os.path.isdir(url):
      url = CheckOutput(["git", "config", "remote.origin.url"], url).strip()
    url = urlsplit(url)
    if url.netloc != "git.launchpad.net":
      print("Unexpected origin %s" % url.netloc, file=sys.stderr)
      sys.exit(1)
    url = url._replace(scheme="git+ssh")
    if userid:
      url = url._replace(netloc="%s@%s" % (userid, url.netloc))
    CheckCall(["git", "remote", "set-url", "--push", "origin", url.geturl()], path)

class Options(OptionParser):
  def __init__(self):
    OptionParser.__init__(self)

    self.add_option("-c", "--cache-dir", dest="cache_dir",
                    help="Specify a directory for a local mirror")
    self.add_option("-f", "--force", dest="force", action="store_true",
                    help="Force an update")
    self.add_option("--add-pushurls", dest="add_pushurls", action="store_true",
                    help="Add a push URL to the remote for Launchpad "
                         "maintained GIT repositories")
    self.add_option("--lp-userid", dest="lp_userid",
                    help="Specify your launchpad user ID for pushing to GIT")
    self.add_option("--add-upstream-remotes", dest="add_upstream_remotes",
                    action="store_true",
                    help="Add upstream remotes for Launchpad forked GIT "
                         "repositories")

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
  if options.lp_userid and not options.add_pushurls:
    print("--lp-userid must be used with --add-pushurls", file=sys.stderr)
    sys.exit(1)

  c = Config(options)

  PrepareDepotTools()
  RunMigration()

  if options.force or NeedsChromiumSync(c):
    SyncChromium(c, options.force)

  if options.add_upstream_remotes:
    AddUpstreamRemotes()

  if options.add_pushurls:
    AddGitPushUrls(options.lp_userid)

if __name__ == "__main__":
  main()
