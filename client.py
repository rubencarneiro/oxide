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
from oxide_utils import CheckCall, CheckOutput, CHROMIUMDIR, CHROMIUMSRCDIR, TOPSRCDIR
from patch_utils import SyncablePatchSet, SyncError

DEPOT_TOOLS = "https://chromium.googlesource.com/chromium/tools/depot_tools.git"
DEPOT_TOOLS_REV = "d9839e8e7a078df40b33b76bf25c4a2f3199d3be"

GCLIENTFILE = os.path.join(TOPSRCDIR, "gclient.conf")
GCLIENTFILECOPY = os.path.join(CHROMIUMDIR, ".gclient")
DEPOTTOOLSPATH = os.path.join(CHROMIUMDIR, "depot_tools")

def get_svn_info(repo):
  for line in CheckOutput(["svn", "info", repo]).split("\n"):
    m = re.match(r'^([^:]*): (.*)', line.strip())
    if not m: continue
    if m.group(1) == "URL":
      url = m.group(2)
    elif m.group(1) == "Revision":
      rev = m.group(2)

  return (url, rev)

def get_wanted_info_from_gclient_config():
  from gclient_utils import SplitUrlRevision
  with open(GCLIENTFILE, "r") as fd:
    gclient_dict = {}
    exec(fd.read(), gclient_dict)
    assert "solutions" in gclient_dict
    assert len(gclient_dict["solutions"]) == 1
    assert gclient_dict["solutions"][0]["name"] == "src"
    return SplitUrlRevision(gclient_dict["solutions"][0]["url"])

def prepare_depot_tools():
  if not os.path.isdir(DEPOTTOOLSPATH):
    CheckCall(["git", "clone", DEPOT_TOOLS, DEPOTTOOLSPATH])
 
  CheckCall(["git", "pull", "origin", "master"], DEPOTTOOLSPATH)
  CheckCall(["git", "checkout", DEPOT_TOOLS_REV], DEPOTTOOLSPATH)

  sys.path.insert(0, DEPOTTOOLSPATH)
  os.environ["PATH"] = DEPOTTOOLSPATH + ":" + os.getenv("PATH")

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

def check_chromium_status():
  # Force full sync if running "svn info" fails
  try:
    CheckCall(["svn", "info"], CHROMIUMSRCDIR)
  except:
    return (True, True)

  need_sync = False
  # Need a sync if there is no .hg folder
  if not os.path.isdir(os.path.join(CHROMIUMSRCDIR, ".hg")):
    need_sync = True

  # Don't try a sync if we have a .hg folder and the gclient file hasn't changed
  if (not need_sync and os.path.isfile(GCLIENTFILECOPY) and
      os.stat(GCLIENTFILE).st_mtime <= os.stat(GCLIENTFILECOPY).st_mtime):
    return (False, False)

  (cur_url, cur_rev) = get_svn_info(CHROMIUMSRCDIR)
  (wanted_url, wanted_rev) = get_wanted_info_from_gclient_config()

  # Force full sync if the branch has changed
  if wanted_url != cur_url:
    return (True, True)

  if need_sync:
    return (True, False)

  if wanted_rev is '':
    (dummy, wanted_rev) = get_svn_info(wanted_url)

  if wanted_rev == cur_rev:
    return (False, False)

  return (True, False)

def sync_chromium():
  if os.path.isdir(os.path.join(CHROMIUMSRCDIR, ".hg")):
    SyncablePatchSet().hg_patches.unapply_all()
    shutil.rmtree(os.path.join(CHROMIUMSRCDIR, ".hg"))
    os.remove(os.path.join(CHROMIUMSRCDIR, ".hgignore"))

  # Don't use the gclient shell wrapper, as it updates depot_tools
  CheckCall([sys.executable, os.path.join(DEPOTTOOLSPATH, "gclient.py"),
             "sync", "-D"], CHROMIUMDIR)

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

  (need_sync, full_sync) = check_chromium_status()

  shutil.copyfile(GCLIENTFILE, GCLIENTFILECOPY)
  shutil.copystat(GCLIENTFILE, GCLIENTFILECOPY)

  if full_sync and os.path.isdir(CHROMIUMSRCDIR):
    shutil.rmtree(CHROMIUMSRCDIR)
  if need_sync:
    sync_chromium()

  sync_chromium_patches()

if __name__ == "__main__":
  main()
