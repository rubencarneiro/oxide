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

import base64
import hashlib
import os
import os.path
import re
import shutil
from StringIO import StringIO
from subprocess import Popen, CalledProcessError, PIPE

TOPSRCDIR = os.path.abspath(os.path.join(__file__, os.pardir, os.pardir, os.pardir))
CHROMIUMDIR = os.path.join(TOPSRCDIR, "chromium")
CHROMIUMSRCDIR = os.path.join(CHROMIUMDIR, "src")

def GetFileChecksum(file):
  """Return a SHA256 hash from the contents of the specified filename"""
  h = hashlib.sha256()
  with open(file, "r") as fd:
    h.update(fd.read())
  return base64.b16encode(h.digest())

def CheckCall(args, cwd=None, want_stdout=False):
  p = Popen(args, cwd=cwd, stdout=(PIPE if want_stdout == True else None))
  r = p.wait()
  if r is not 0: raise CalledProcessError(r, args)
  if want_stdout: return p.stdout

class PatchSeriesChecksums(dict):
  def __init__(self):
    self.refresh()

  def refresh(self):
    self.clear()

    series = os.path.join(CHROMIUMSRCDIR, ".hg", "patches", "series.checksums")
    if not os.path.isfile(series):
      return

    with open(series, "r") as fd:
      for line in fd.readlines():
        m = re.match(r'([^:]*):(.*)', line.strip())
        self[m.group(1)] = m.group(2)

class Patch:
  """This class represents a patch in a patch series"""

  def __init__(self, filename, checksum):
    self.filename = filename
    self.checksum = checksum
    self.applied = False

class PatchSeries(object):
  """Class to represents a series of patches"""

  def __init__(self, patchdir):
    self.patchdir = patchdir
    self.refresh()

  def refresh(self):
    self._patches = []

    series = os.path.join(self.patchdir, "series")
    if not os.path.isfile(series):
      return

    with open(series, "r") as fd:
      for line in fd.readlines():
        if line.strip().startswith("#"):
          continue
        filename = line.strip()
        checksum = GetFileChecksum(os.path.join(self.patchdir, filename))
        self._patches.append(Patch(filename, checksum))

  def __getitem__(self, key):
    """Look up patch by index or filename"""
    if type(key) == str:
      for patch in self:
        if patch.filename == key:
          return patch
      raise KeyError

    return self._patches[key]

  def __len__(self):
    return len(self._patches)

  def __contains__(self, value):
    """Determine whether this series contains the specified patch"""
    for patch in self:
      if patch.filename == value.filename:
        return True
    return False

class HgPatchSeries(PatchSeries):
  """This class represents a series of Chromium patches stored in the Chromium
  checkout. These are kept in a Mercurial patch queue, not in revision control,
  and are the ones that are actually applied to the Chromium checkout
  """
  def __init__(self):
    super(HgPatchSeries, self).__init__(os.path.join(CHROMIUMSRCDIR, ".hg", "patches"))

  def refresh(self):
    super(HgPatchSeries, self).refresh()
    self._update_status()

  @property
  def top_index(self):
    """Return the index of the top-most patch"""
    for i in range(len(self)):
      if not self[i].applied:
        return i - 1
    return len(self) - 1

  @top_index.setter
  def top_index(self, index):
    """Set the index of the top-most patch. If the new index is larger
    than the current index, additional patches are pushed from the queue.
    If the new index is lower than the current index, then patches are
    popped from the queue. An index of -1 unapplies all patches
    """
    if index == self.top_index:
      return

    if index > self.top_index:
      CheckCall(["hg", "qpush", str(index)], CHROMIUMSRCDIR)
    else:
      CheckCall(["hg", "qpop", str(index) if index > -1 else "-a"],
                CHROMIUMSRCDIR)

    self._update_status()

  def _update_status(self):
    applied = set()
    status = os.path.join(self.patchdir, "status")
    if os.path.isfile(status):
      with open(status, "r") as fd:
        for line in fd.readlines():
          applied.add(re.match(r'[^:]*:(.*)', line.strip()).group(1))

    for patch in self:
      patch.applied = patch.filename in applied
      try:
        applied.remove(patch.filename)
      except:
        pass

    assert len(applied) is 0

class SourcePatchSeries(PatchSeries):
  """This class represents a Chromium patch series in revision control.
  These patches aren't applied directly to the Chromium checkout,
  but are imported in to a Mercurial patch queue and applied from there.
  This provides a mechanism for patches to be modified / refreshed,
  and allows the Oxide source branch to be updated without breaking
  your local Chromium checkout
  """
  def __init__(self):
    super(SourcePatchSeries, self).__init__(os.path.join(TOPSRCDIR, "patches"))

class SyncablePatch:
  """This class represents a resolved patch after doing a sync"""

  def __init__(self, filename, checksum, copy_source):
    self.filename = filename
    self.checksum = checksum
    self.copy_source = copy_source

class PatchSyncResult:
  """This class represents the results of doing a sync"""

  def __init__(self):
    self.patches = []
    self.src_removal_list = []
    self.hg_removal_list = []

class SyncablePatchSet:
  """This class represents a syncable patch set, consisting of a master
  series of patches in revision control, and a copy of this series in the
  Chromium source checkout
  """
  def __init__(self, src = SourcePatchSeries(),
               hg = HgPatchSeries(),
               checksums = PatchSeriesChecksums()):
    self.src_patches = src
    self.hg_patches = hg
    self.checksums = checksums
    self.result = None

  def prepare_sync(self):
    try:
      self._prepare_sync_internal()
    except:
      self.result = None
      raise

  def _prepare_sync_internal(self):
    self.result = PatchSyncResult()

    # Determine how far back to unwind the applied patch queue
    new_top_index = -1
    for i in range(len(self.hg_patches)):
      hg_patch = self.hg_patches[i]
      try:
        src_patch = self.src_patches[i]
      except:
        src_patch = None

      if (src_patch is not None and
          (src_patch.filename != hg_patch.filename or
           src_patch.checksum != hg_patch.checksum)):
        break

      new_top_index = i

    if new_top_index < self.hg_patches.top_index:
      self.hg_patches.top_index = new_top_index

    # Iterate over the patch series in revision control
    for patch in self.src_patches:
      if patch in self.hg_patches:
        # Patch exists already in Chromium
        hg_patch = self.hg_patches[patch.filename]
        if patch.checksum == hg_patch.checksum:
          # Both copies of the patch are the same. Just add to the result queue
          self.result.patches.append(SyncablePatch(patch.filename,
                                                   patch.checksum,
                                                   "none"))
          continue

        try:
          last_sync_checksum = self.checksums[patch.filename]
        except:
          raise Exception("Patch %s in your Chromium checkout is not the "
                          "same as the copy in revision control. As it is "
                          "not possible to determine which copy to keep, "
                          "this will need to be resolved manually. You may "
                          "see this error if your Chromium checkout was "
                          "set up using a revision of Oxide before r238" %
                          patch.filename)

        if (patch.checksum != last_sync_checksum and
            hg_patch.checksum != last_sync_checksum):
          # Both copies of the patch have been modified. Resolve manually
          raise Exception("Patch %s has been modified both in revision "
                          "control and in your Chromium checkout. This will "
                          "need to be resolved manually" % patch.filename)

        if patch.checksum != last_sync_checksum:
          # The copy in revision control has changed, so we need to
          # copy it to Chromium
          assert not hg_patch.applied
          self.result.patches.append(SyncablePatch(patch.filename,
                                                   patch.checksum,
                                                   "bzr"))
          continue

        assert hg_patch.checksum != last_sync_checksum

        # The copy in Chromium has changed, so we need to copy it back to
        # revision control
        self.result.patches.append(SyncablePatch(patch.filename,
                                                 hg_patch.checksum,
                                                 "hg"))

      else:
        # Patch doesn't exist in Chromium
        if patch.filename not in self.checksums:
          # Patch has been added to revision control, so copy it to Chromium
          self.result.patches.append(SyncablePatch(patch.filename,
                                                   patch.checksum,
                                                   "bzr"))
        else:
          # Patch has been removed from Chromium, so remove it from revision
          # control
          self.result.src_removal_list.append(patch.filename)

    # Iterate over the patch series in the Chromium checkout
    for i in range(len(self.hg_patches)):
      patch = self.hg_patches[i]
      if patch in self.src_patches:
        # We've already dealt with this patch
        continue

      if patch.filename not in self.checksums:
        # Patch has been added to Chromium, so copy it back to revision control
        prev_patch = self.hg_patches[i - 1] if i > 0 else None
        next_patch = None
        for j in range(i + 1, len(self.hg_patches)):
          if self.hg_patches[j] in self.src_patches:
            next_patch = self.hg_patches[j]
            break

        insertion_index = None

        for j in range(len(self.result.patches) + 1):
          p = self.result.patches[j - 1] if j > 0 else None
          n = self.result.patches[j] if j < len(self.result.patches) else None
          if (((p is None and prev_patch is None) or
               (p is not None and prev_patch is not None and
                p.filename == prev_patch.filename)) and
              ((n is None and next_patch is None) or
               (n is not None and next_patch is not None and
                n.filename == next_patch.filename))):
            insertion_index = j
            break

        if not insertion_index:
          raise Exception("Unable to determine where to insert %s from your "
                          "Chromium checkout in to the patch series in "
                          "revision control. This will need to be resolved "
                          "manually" % patch.filename)

        self.result.patches.insert(insertion_index,
                                   SyncablePatch(patch.filename,
                                                 patch.checksum,
                                                 "hg"))
      else:
        # Patch has been removed from revision control, so remove it from
        # Chromium
        self.result.hg_removal_list.append(patch.filename)

  def do_sync(self):
    if not self.result:
      raise Exception("prepare_sync() needs to complete successfully before "
                      "do_sync() is called")

    series_content = StringIO()
    checksum_content = StringIO()

    for patch in self.result.patches:
      series_content.write("%s\n" % patch.filename)
      checksum_content.write("%s:%s\n" % (patch.filename, patch.checksum))
      if patch.copy_source == "bzr":
        shutil.copy2(os.path.join(TOPSRCDIR, "patches", patch.filename),
                     os.path.join(CHROMIUMSRCDIR, ".hg", "patches"))
      elif patch.copy_source == "hg":
        shutil.copy2(os.path.join(CHROMIUMSRCDIR, ".hg", "patches", patch.filename),
                     os.path.join(TOPSRCDIR, "patches"))
      else:
        assert patch.copy_source == "none"

    series_content.seek(0)
    with open(os.path.join(TOPSRCDIR, "patches", "series"), "w") as fd:
      fd.write(series_content.read())
    series_content.seek(0)
    with open(os.path.join(CHROMIUMSRCDIR, ".hg", "patches", "series"), "w") as fd:
      fd.write(series_content.read())
    checksum_content.seek(0)
    with open(os.path.join(CHROMIUMSRCDIR, ".hg", "patches", "series.checksums"), "w") as fd:
      fd.write(checksum_content.read())

    for filename in self.result.src_removal_list:
      os.remove(os.path.join(TOPSRCDIR, "patches", filename))
    for filename in self.result.hg_removal_list:
      os.remove(os.path.join(CHROMIUMSRCDIR, ".hg", "patches", filename))

    self.refresh()

  def refresh(self):
    self.src_patches.refresh()
    self.hg_patches.refresh()
    self.checksums.refresh()
