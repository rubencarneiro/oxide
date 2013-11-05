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
import re
import shutil
import sys
from urlparse import urljoin, urlsplit

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "build", "python"))
from oxide_utils import CheckCall, SyncablePatchSet, CHROMIUMDIR, CHROMIUMSRCDIR, TOPSRCDIR

DEPOT_TOOLS = "https://chromium.googlesource.com/chromium/tools/depot_tools.git"
DEPOT_TOOLS_REV = "3b5efdf64d35ba20f8afd4f568eb190c3f1e8e12"

gclientfile = os.path.join(TOPSRCDIR, "gclient.conf")

def get_svn_info(repo):
  for line in CheckCall(["svn", "info", repo], want_stdout=True).readlines():
    m = re.match(r'^([^:]*): (.*)', line.strip())
    if not m: continue
    if m.group(1) == "URL":
      url = m.group(2)
    elif m.group(1) == "Revision":
      rev = m.group(2)

  return (url, rev)

def get_wanted_info_from_gclient_config():
  with open(gclientfile, "r") as fd:
    gclient_dict = {}
    exec(fd.read(), gclient_dict)
    assert "solutions" in gclient_dict
    assert len(gclient_dict["solutions"]) == 1
    assert gclient_dict["solutions"][0]["name"] == "src"
    gclient_url = urlsplit(gclient_dict["solutions"][0]["url"])
    m = re.match(r'([^@]*)@?(.*)', gclient_url.path)

    return (urljoin(gclient_url.geturl(), m.group(1)), m.group(2))

def prepare_depot_tools():
  depot_tools_path = os.path.join(CHROMIUMDIR, "depot_tools")

  if not os.path.isdir(depot_tools_path):
    check_call(["git", "clone", DEPOT_TOOLS, depot_tools_path])
 
  CheckCall(["git", "pull", "origin", "master"], depot_tools_path)
  CheckCall(["git", "checkout", DEPOT_TOOLS_REV], depot_tools_path)

  os.environ["PATH"] = os.path.join(CHROMIUMDIR, "depot_tools") + ":" + os.getenv("PATH")

def ensure_patch_consistency(patchset):
  for patch in patchset.hg_patches:
    if patch.filename in patchset.checksums:
      if patch.checksum == patchset.checksums[patch.filename]:
        continue

    if (patch in patchset.src_patches and
        patch.checksum == patchset.src_patches[patch.filename].checksum):
      continue

    raise Exception("Patch %s in your Chromium source checkout has been "
                    "modified. Please resolve this manually. Note, you may "
                    "see this error if your Chromium checkout was set up "
                    "using a revision of Oxide before r238" % patch.filename)

def need_chromium_sync():
  try:
    CheckCall(["svn", "info"], CHROMIUMSRCDIR)
  except:
    return True

  (cur_url, cur_rev) = get_svn_info(CHROMIUMSRCDIR)
  (wanted_url, wanted_rev) = get_wanted_info_from_gclient_config()

  if wanted_url != cur_url:
    raise Exception("The URL specified in the gclient config doesn't match " +
                    "the current URL")

  if wanted_rev is '':
    (dummy, wanted_rev) = get_svn_info(wanted_url)

  return wanted_rev != cur_rev

def sync_chromium():
  if os.path.isdir(os.path.join(CHROMIUMSRCDIR, ".hg")):
    shutil.rmtree(os.path.join(CHROMIUMSRCDIR, ".hg"))
    os.remove(os.path.join(CHROMIUMSRCDIR, ".hgignore"))

  CheckCall(["gclient", "sync", "--force", "--gclientfile", gclientfile],
            CHROMIUMDIR)

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
  CheckCall(["hg", "init"], CHROMIUMSRCDIR)
  CheckCall(["hg", "addremove"], CHROMIUMSRCDIR)
  CheckCall(["hg", "ci", "-m", "Base checkout with client.py"], CHROMIUMSRCDIR)
  CheckCall(["hg", "qinit"], CHROMIUMSRCDIR)

def sync_chromium_patches(patchset):
  patchset.prepare_sync()
  patchset.do_sync()

def apply_chromium_patches(patchset):
  patchset.hg_patches.top_index = len(patchset.hg_patches) - 1

def unapply_chromium_patches(patchset):
  patchset.hg_patches.top_index = -1

def main():
  prepare_depot_tools()

  patchset = SyncablePatchSet()
  ensure_patch_consistency(patchset)

  if need_chromium_sync():
    unapply_chromium_patches(patchset)
    sync_chromium()
    patchset.refresh()

  sync_chromium_patches(patchset)
  apply_chromium_patches(patchset)

if __name__ == "__main__":
  main()
