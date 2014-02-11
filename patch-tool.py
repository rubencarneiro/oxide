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
import optparse
import os.path
import shutil
from StringIO import StringIO
import sys

sys.dont_write_bytecode = True
os.environ["PYTHONDONTWRITEBYTECODE"] = "1"

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "build", "python"))
from oxide_utils import CheckCall, GetFileChecksum, CHROMIUMDIR, CHROMIUMSRCDIR, TOPSRCDIR
from patch_utils import HgPatchSeries, OldPatchSeries, SourcePatchSeries, SyncablePatchSet, SyncError
import subcommand

class OptionParser(optparse.OptionParser):
  def __init__(self):
    optparse.OptionParser.__init__(self)

    self.description = """
Tool for managing a queue of Chromium patches in Oxide.
"""

@subcommand.Command("sync", " [patchname]")
@subcommand.CommandOption("-n", "--dry-run", dest="dry_run", action="store_true",
                          help="Display the changes that would be made without "
                               "actually modifying anything")
def cmd_sync(options, args):
  """Synchronize the patch queues in Bzr and Chromium.

  This command attempts to synchronize the patch queues in Bzr and Chromium.
  Firstly, a 3-way merge of the series files is attempted. The active patches
  listed in the result of this merge are then iterated over. Any patches that
  have been modified in Bzr are copied to Chromium. Any patches that have been
  modified in Chromium are copied back to Bzr.

  Patches that are currently applied to your Chromium checkout are
  automatically unapplied where necessary.
  """

  s = SyncablePatchSet()
  try:
    s.calculate_sync()
    if not options.dry_run:
      s.do_sync()
      return
  except SyncError as e:
    print(e, file=sys.stderr)
    sys.exit(1)

  unapply_to = None
  if s.result.unapply_to is not None:
    unapply_to = s.hg_patches[s.result.unapply_to]
  if unapply_to != s.hg_patches.top_patch:
    if not unapply_to:
      print("The Chromium patch queue will be fully unwound")
    elif unapply_to.applied:
      print("The Chromium patch queue will be unwound to %s" %
            s.result.unapply_to)
    else:
      print("The Chromium patch queue will not be unwound")
  else:
    print("The Chromium patch queue will not be unwound")

  width = max(len(patch.filename) for patch in s.result.patches)
  print("\nFinal patch set:")
  print("".join((" %-*s%s\n" %
      (width, patch.filename,
      " (copy from Chromium)" if patch.result == "use-hg" else
      " (copy from Bzr)" if patch.result == "use-bzr" else
      "")) for patch in s.result.patches if patch.active))

  width = max(len(line) for line in s.result.patches.contents.splitlines())
  print("\nSeries file contents:\n")
  print("="*width)
  print("%s" % s.result.patches.contents)
  print("="*width)

  copy_list = [patch for patch in s.result.patches
               if patch.result == "use-bzr" or patch.result == "use-hg"]
  if len(copy_list) == 0:
    print("\nNo files to be copied")
  else:
    print("\nFiles to be copied:")
    for patch in copy_list:
      if patch.result == "use-bzr":
        src = os.path.join(s.src_patches.patchdir, patch.filename)
        dst = os.path.join(s.hg_patches.patchdir, patch.filename)
      else:
        src = os.path.join(s.hg_patches.patchdir, patch.filename)
        dst = os.path.join(s.src_patches.patchdir, patch.filename)
      print(" %s ==>\n   %s" % (src, dst))

  removal_list = s.result.files_to_remove
  if len(removal_list) == 0:
    print("\nNo files to be deleted")
  else:
    print("\nFiles to be deleted:")
    print("".join((" %s\n" % patch) for patch in removal_list))

@subcommand.Command("status", " <patchname>")
def cmd_status(options, args):
  """Display the status of a patch in both patch queues"""
  if len(args) is not 1:
    print("You must specify the filename of one patch", file=sys.stderr)
    sys.exit(1)

  # XXX: Handle full paths?
  filename = args[0]

  s = SourcePatchSeries()
  h = HgPatchSeries()
  o = OldPatchSeries()

  src_patch = s[filename] if filename in s else None
  hg_patch = h[filename] if filename in h else None
  old_patch = o[filename] if filename in o else None

  if not src_patch and not hg_patch and not old_patch:
    print("The specified patch name '%s' cannot be found" % filename,
          file=sys.stderr)
    sys.exit(1)

  print("Status of '%s':"  % filename)

  def get_status(patch):
    status = ""
    if patch:
      if not old_patch:
        status = "new"
      elif old_patch.checksum != patch.checksum:
        status = "modified"
      else:
        status = "unmodified"
      status += " (active)" if patch.active else " (inactive)"
    else:
      if old_patch:
        status = "deleted"
      else:
        status = "missing"
    return status

  print("Bzr:      %s" % get_status(src_patch))
  print("Chromium: %s" % get_status(hg_patch))

@subcommand.Command("conflicts")
def cmd_conflicts(options, args):
  """Display a list of conflicts.

  This command outputs a list of patches that "sync" cannot automatically
  resolve.
  """

  try:
    s = SyncablePatchSet()
    s.calculate_sync()
  except SyncError:
    pass
  else:
    print("There are no conflicts")
    return

  if not s.result:
    print("The series files could not be merged, please see the results of "
          "the attempted merge below:")
    try:
      print("")
      CheckCall(["diff3", "--merge",
                 s.src_patches.series,
                 os.path.join(s.hg_patches.patchdir, ".series.orig"),
                 s.hg_patches.series])
      print("")
    except:
      pass
    return

  print("The following patches have been modified in both Chromium and Bzr, "
        "so it is not possible to determine which copy to keep:")
  print("".join((" %s\n" % conflict) for conflict in s.result.conflicts))

@subcommand.Command("resolve", " [patchname]")
@subcommand.CommandOption("--use-bzr", dest="use_bzr", action="store_true",
                          help="Save the copy of the specified patch that is "
                               "stored in Bzr")
@subcommand.CommandOption("--use-chromium", dest="use_chromium",
                          action="store_true",
                          help="Save the copy of the specified patch that is "
                               "stored in your Chromium checkout")
def cmd_resolve(options, args):
  """Resolve a conflict.

  Manually specify a resolution for a conflict.
  """

  if not options.use_bzr and not options.use_chromium:
    print("You must specify either --use-bzr or --use-chromium",
          file=sys.stderr)
    sys.exit(1)

  if len(args) != 1:
    print("Please specify the name of one patch", file=sys.stderr)
    sys.exit(1)

  try:
    s = SyncablePatchSet()
    s.calculate_sync()
  except:
    pass
  else:
    print("There are no conflicts to resolve", file=sys.stderr)
    sys.exit(1)

  if not s.result:
    print("This tool can currently only resolve conflicts in individual "
          "patches", file=sys.stderr)
    sys.exit(1)

  patch = None
  for filename in s.result.conflicts:
    if filename != args[0]:
      continue

    patch = s.result.patches[filename]
    break

  if not patch:
    print("The specified patch was not in the list of conflicts",
          file=sys.stderr)
    sys.exit(1)

  if options.use_chromium:
    src_file = os.path.join(s.hg_patches.patchdir, filename)
    dst = s.src_patches.patchdir
  else:
    assert options.use_bzr
    src_file = os.path.join(s.src_patches.patchdir, filename)
    dst = s.hg_patches.patchdir
    hg_patch = s.hg_patches[filename] if filename in s.hg_patches else None
    if hg_patch.applied:
      index = s.hg_patches.index(hg_patch) - 1
      s.hg_patches.top_patch = None if index == -1 else s.hg_patches[index]

  shutil.copy2(src_file, dst)
  new_checksum = GetFileChecksum(src_file)

  checksum_content = StringIO()
  for patch in s.old_patches:
    checksum_content.write("%s:%s\n" %
        (patch.filename,
         patch.checksum if patch.filename != filename else new_checksum))

  checksum_content.seek(0)
  with open(os.path.join(s.hg_patches.patchdir,
                         ".series.checksums"), "w") as fd:
    fd.write(checksum_content.read())

def main():
  return subcommand.Dispatcher.execute(sys.argv[1:], OptionParser())

if __name__ == "__main__":
  sys.exit(main())
