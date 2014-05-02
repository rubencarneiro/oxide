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

from __future__ import print_function
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
from patch_utils import SyncablePatchSet, SyncError

DEPOT_TOOLS_GIT_URL = "https://chromium.googlesource.com/chromium/tools/depot_tools.git"
DEPOT_TOOLS_GIT_REV = "3f802b8b1f20b44a54f2c32a9e721ac82c16c472"
DEPOT_TOOLS_PATH = os.path.join(TOPSRCDIR, "third_party", "depot_tools")
DEPOT_TOOLS_OLD_PATH = os.path.join(TOPSRCDIR, "chromium", "depot_tools")

CHROMIUM_RELEASE_DEPS_PATH = os.path.join(os.path.dirname(CHROMIUMSRCDIR), "release_deps")

CHROMIUM_SVN_URL = "http://src.chromium.org/chrome/releases/%s"
CHROMIUM_GCLIENT_SPEC = (
  "solutions = ["
    "{ \"name\": \"release_deps\", "
      "\"url\": \"%s\", "
      "\"deps_file\": \"DEPS\", "
      "\"managed\": True, "
      "\"custom_deps\": "
        "{ \"build\": None, "
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
          "\"src/chrome_frame/tools/test/reference_build/chrome_win\": None, "
          "\"src/chrome/tools/test/reference_build/chrome_linux\": None, "
          "\"src/chrome/tools/test/reference_build/chrome_mac\": None, "
          "\"src/third_party/hunspell_dictionaries\": None, "
          "\"src/third_party/WebKit/LayoutTests\": None, "
          "\"src/webkit/data/layout_tests/LayoutTests\": None, "
          "\"src/win8\": None, "
        "}, "
      "\"safesync_url\": \"\", "
    "} "
  "]"
)
CHROMIUM_GCLIENT_FILE = os.path.join(os.path.dirname(CHROMIUMSRCDIR), ".gclient")

def is_svn_repo(repo):
  try:
    CheckCall(["svn", "info"], repo)
  except:
    return False
  return True

def get_svn_info(repo):
  for line in CheckOutput(["svn", "info", repo]).split("\n"):
    m = re.match(r'^([^:]*): (.*)', line.strip())
    if not m: continue
    if m.group(1) == "URL":
      url = m.group(2)
    elif m.group(1) == "Revision":
      rev = m.group(2)

  return (url, rev)

def is_git_repo(repo):
  try:
    CheckCall(["git", "status"], repo)
  except:
    return False
  return True

def checkout_git_repo(repo, path, commit):
  if not is_git_repo(path):
    if os.path.isdir(path):
      shutil.rmtree(path)
    CheckCall(["git", "clone", repo, path])

  CheckCall(["git", "fetch"], path)
  CheckCall(["git", "checkout", commit], path)

def get_chromium_version():
  with open(os.path.join(TOPSRCDIR, "CHROMIUM_VERSION"), "r") as fd:
    return fd.read().strip()

def get_chromium_svn_url():
  return CHROMIUM_SVN_URL % get_chromium_version()

def get_chromium_gclient_spec():
  return CHROMIUM_GCLIENT_SPEC % get_chromium_svn_url()

def prepare_depot_tools():
  if is_git_repo(DEPOT_TOOLS_OLD_PATH) and not is_git_repo(DEPOT_TOOLS_PATH):
    os.rename(DEPOT_TOOLS_OLD_PATH, DEPOT_TOOLS_PATH)

  checkout_git_repo(DEPOT_TOOLS_GIT_URL, DEPOT_TOOLS_PATH, DEPOT_TOOLS_GIT_REV)

  sys.path.insert(0, DEPOT_TOOLS_PATH)
  os.environ["PATH"] = DEPOT_TOOLS_PATH + ":" + os.getenv("PATH")

def ensure_patch_consistency():
  patchset = SyncablePatchSet()
  for patch in patchset.hg_patches:
    if (patch in patchset.old_patches and
        patch.checksum == patchset.old_patches[patch.filename].checksum):
      continue

    # For pre-r238 checkouts
    if (patch in patchset.src_patches and
        patch.checksum == patchset.src_patches[patch.filename].checksum):
      continue

    print("Patch %s in your Chromium source checkout has been "
          "modified. Please resolve this manually. Note, you may "
          "see this error if your Chromium checkout was created "
          "using a revision of Oxide before r238" % patch.filename,
          file=sys.stderr)
    sys.exit(1)

def needs_chromium_sync():
  release_repo = os.path.join(os.path.dirname(CHROMIUMSRCDIR), "release_deps")
  # Need to sync if there is no release deps svn repo
  if not is_svn_repo(CHROMIUM_RELEASE_DEPS_PATH):
    return True

  # Need to sync for a new release
  if get_svn_info(CHROMIUM_RELEASE_DEPS_PATH)[0] != get_chromium_svn_url():
    return True

  # Need to sync if there is no svn repo
  if not is_svn_repo(CHROMIUMSRCDIR):
    return True

  # Need a sync if there is no .hg folder
  if not os.path.isdir(os.path.join(CHROMIUMSRCDIR, ".hg")):
    return True

  # Sync if there is no .gclient file
  if not os.path.isfile(CHROMIUM_GCLIENT_FILE):
    return True

  # Sync if the gclient spec has changed
  if (GetFileChecksum(CHROMIUM_GCLIENT_FILE) !=
      GetChecksum(get_chromium_gclient_spec())):
    return True

  return False

def sync_chromium():
  if os.path.isdir(os.path.join(CHROMIUMSRCDIR, ".hg")):
    SyncablePatchSet().hg_patches.unapply_all()
    shutil.rmtree(os.path.join(CHROMIUMSRCDIR, ".hg"))
    os.remove(os.path.join(CHROMIUMSRCDIR, ".hgignore"))

  chromium_dir = os.path.dirname(CHROMIUMSRCDIR)
  if not os.path.isdir(chromium_dir):
    os.makedirs(chromium_dir)

  if (is_svn_repo(CHROMIUM_RELEASE_DEPS_PATH) and
      get_svn_info(CHROMIUM_RELEASE_DEPS_PATH)[0] != get_chromium_svn_url()):
    shutil.rmtree(CHROMIUM_RELEASE_DEPS_PATH)

  # Don't use the gclient shell wrapper, as it updates depot_tools
  CheckCall([sys.executable, os.path.join(DEPOT_TOOLS_PATH, "gclient.py"),
             "config", "--spec", get_chromium_gclient_spec()], chromium_dir)
  CheckCall([sys.executable, os.path.join(DEPOT_TOOLS_PATH, "gclient.py"),
             "sync", "-D"], chromium_dir)

  with open(os.path.join(CHROMIUMSRCDIR, ".hgignore"), "w") as f:
    f.write("~$\n")
    f.write("\.svn/\n")
    f.write("\.git/\n")
    f.write("^out/\n")
    f.write("\.host\.(.*\.|)mk$\n")
    f.write("\.target\.(.*\.|)mk$\n")
    f.write("Makefile(\.*|)$\n")
    f.write("^\.hgignore$\n")
    f.write("\.pyc$\n")
    f.write("\.tmp$\n")
  CheckCall(["hg", "init"], CHROMIUMSRCDIR)
  hgrc = os.path.join(CHROMIUMSRCDIR, ".hg", "hgrc")
  if not os.path.isfile(hgrc):
    with open(hgrc, "w") as f:
      f.write("[ui]\n")
      f.write("username = oxide\n\n")
      f.write("[extensions]\n")
      f.write("mq =\n")
  CheckCall(["hg", "addremove"], CHROMIUMSRCDIR)
  CheckCall(["hg", "ci", "-m", "Base checkout with client.py"], CHROMIUMSRCDIR)
  CheckCall(["hg", "qinit"], CHROMIUMSRCDIR)

def sync_chromium_patches():
  patchset = SyncablePatchSet()
  try:
    patchset.calculate_sync()
    patchset.do_sync()
    patchset.hg_patches.apply_all()
  except SyncError as e:
    print(e, file=sys.stderr)
    sys.exit(1)

def main():
  prepare_depot_tools()

  ensure_patch_consistency()

  old_chromium_dir = os.path.join(TOPSRCDIR, "chromium")
  if os.path.isdir(old_chromium_dir):
    shutil.rmtree(old_chromium_dir)

  if needs_chromium_sync():
    sync_chromium()

  sync_chromium_patches()

if __name__ == "__main__":
  main()
