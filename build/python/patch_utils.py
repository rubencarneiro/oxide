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
from oxide_utils import CheckCall, CheckOutput, GetFileChecksum, CHROMIUMSRCDIR, TOPSRCDIR
import re
import shutil
from StringIO import StringIO

class SyncError(Exception):
  pass

class Patch:
  """This class represents a patch in a patch series"""

  def __init__(self, filename, checksum, active = False):
    self.filename = filename
    self.checksum = checksum
    self.active = active
    self.applied = False

class PatchList(object):
  """A iterable list-like container of patches. Patches can be looked up
  by index or filename
  """

  def __init__(self):
    self._patches = []

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
    filename = value if type(value) == str else value.filename
    for patch in self:
      if patch.filename == filename:
        return True
    return False

  def index(self, value):
    if type(value) == str:
      for patch in self:
        if patch.filename == value:
          value = patch
          break

    return self._patches.index(value)

class PatchSeries(PatchList):
  """This class represents an actual patch series file and
  patch directory
  """

  def __init__(self, patchdir):
    super(PatchSeries, self).__init__()
    self.patchdir = patchdir

    self.refresh()

  @property
  def series(self):
    return os.path.join(self.patchdir, "series")

  def refresh(self):
    self._patches = []

    if os.path.isfile(self.series):
      # Add active patches listed in series file
      with open(self.series, "r") as fd:
        for line in fd.readlines(): 
          if line.strip().startswith("#"):
            continue
          filename = line.strip()
          try:
            checksum = GetFileChecksum(os.path.join(self.patchdir, filename))
          except IOError:
            checksum = None
          self._patches.append(Patch(filename, checksum, True))

    # Add patches not active in the series file as inactive patches
    extra_patchfiles = [f for f in os.listdir(self.patchdir)
                        if os.path.splitext(f)[1] == ".patch" and f not in self]
    for p in extra_patchfiles:
      checksum = GetFileChecksum(os.path.join(self.patchdir, p))
      self._patches.append(Patch(p, checksum, False))

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
  def top_patch(self):
    for i in range(len(self)):
      patch = self[i]
      if not patch.applied:
        return None if i == 0 else self[i - 1]
      assert patch.active

  @top_patch.setter
  def top_patch(self, value):
    if value is not None:
      if value not in self:
        raise Exception("The specified patch is not in this series")

      if type(value) != str:
        value = value.filename

    if value is None:
      CheckCall(["hg", "qpop", "-a"], CHROMIUMSRCDIR)
    else:
      CheckCall(["hg", "qgoto", value], CHROMIUMSRCDIR)

    self._update_status()

  def apply_all(self):
    CheckCall(["hg", "qpush", "-a"], CHROMIUMSRCDIR)

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

class OldPatchSeries(PatchList):
  """This class contains a list of patches and their checksums the last
  time that the patchset was synchronized
  """

  def __init__(self):
    series_old = os.path.join(CHROMIUMSRCDIR, ".hg", "patches",
                              "series.checksums")
    series_new = os.path.join(CHROMIUMSRCDIR, ".hg", "patches",
                              ".series.checksums")
    if not os.path.isfile(series_new) and os.path.isfile(series_old):
      os.rename(series_old, series_new)

    super(OldPatchSeries, self).__init__()

    self.refresh()

  def refresh(self):
    series = os.path.join(CHROMIUMSRCDIR, ".hg", "patches",
                          ".series.checksums")
    if not os.path.isfile(series):
      return

    with open(series, "r") as fd:
      for line in fd.readlines():
        m = re.match(r'([^:]*):(.*)', line.strip())
        self._patches.append(Patch(m.group(1), m.group(2)))

class SyncablePatch:
  """This class represents a resolved patch after doing a sync"""

  def __init__(self, filename, active):
    self.filename = filename
    self.active = active
    self.result = None
    self.conflict_msg = None

class ResultPatchSeries(PatchList):
  def __init__(self):
    super(ResultPatchSeries, self).__init__()
    self._contents = None

  @property
  def contents(self):
    return self._contents

  @contents.setter
  def contents(self, contents):
    self._contents = contents

    patches = []

    for line in self._contents.splitlines():
      if line.strip().startswith("#"):
        continue
      filename = line.strip()
      if filename in self:
        patch = self[filename]
        patch.active = True
        patches.append(patch)
      else:
        patches.append(SyncablePatch(filename, True))

    old_patches = self._patches
    self._patches = patches

    for patch in old_patches:
      if patch.filename not in self:
        patch.active = False
        self._patches.append(patch)

  def write_series(self, filename):
    tmp = filename + ".tmp"
    with open(tmp, "w") as fd:
      fd.write(self.contents)

    os.rename(tmp, filename)

  def append(self, patch):
    self._patches.append(patch)

  def remove(self, patch):
    self._patches.remove(patch)

class PatchSyncResult(object):
  """This class represents the results of doing a sync"""

  def __init__(self, patchset):
    self.patches = ResultPatchSeries()
    self.unapply_to = None
    self._patchset = patchset

  @property
  def conflicts(self):
    return [patch.filename for patch in self.patches if patch.result == "conflicts"]

  @property
  def files_to_remove(self):
    rv = []

    for series in [self._patchset.src_patches, self._patchset.hg_patches]:
      for patch in series:
        if patch not in self.patches:
          rv.append(os.path.join(series.patchdir, patch.filename))

    return rv

class SyncablePatchSet:
  """This class represents a syncable patch set, consisting of a master
  series of patches in revision control, and a copy of this series in the
  Chromium source checkout
  """
  def __init__(self, src = SourcePatchSeries(),
               hg = HgPatchSeries(),
               old_patches = OldPatchSeries()):
    self.src_patches = src
    self.hg_patches = hg
    self.old_patches = old_patches
    self.result = None

  def calculate_sync(self):
    result = PatchSyncResult(self)

    # Do a 3-way merge of series files
    src_file = None
    if (not os.path.isfile(self.hg_patches.series) and
        not os.path.isfile(self.src_patches.series)):
      # Nothing to do
      self.result = result
      return
    elif not os.path.isfile(self.hg_patches.series):
      # Patch series hasn't been created in your Chromium checkout yet
      src_file = self.src_patches.series
    elif not os.path.isfile(self.src_patches.series):
      # Patch series doesn't exist in revision control
      src_file = self.hg_patches.series
    elif not os.path.isfile(os.path.join(self.hg_patches.patchdir,
                                         ".series.orig")):
      # We're missing the original patch, so we can't do a merge
      # If both series files are the same, then that's ok. If not, we're
      # screwed
      if (GetFileChecksum(self.src_patches.series) !=
          GetFileChecksum(self.hg_patches.series)):
        # FIXME: Need to define a workflow for fixing this
        raise Exception("Cannot do a 3-way merge of the series files in "
                        "revision control and your Chromium checkout "
                        "because the original file is missing. This might "
                        "happen if your Chromium checkout was created "
                        "with a revision of Oxide before r241")
      else:
        src_file = self.src_patches.series

    if src_file is not None:
      with open(src_file, "r") as fd:
        result.patches.contents = fd.read()

    if result.patches.contents is None:
      try:
        result.patches.contents = CheckOutput(
            ["diff3", "--merge",
             self.src_patches.series,
             os.path.join(self.hg_patches.patchdir, ".series.orig"),
             self.hg_patches.series])
      except:
        # FIXME: Need to define a workflow for fixing this
        raise SyncError("3-way merge of the series files in revision control "
                        "and your Chromium checkout failed. This will need "
                        "to be resolved manually")

    self.result = result

    # For any patch files currently in Bzr or Chromium but not in the results,
    # add them as inactive patches. Some of these will be removed later
    for series in [self.src_patches, self.hg_patches]:
      for patch in series:
        if patch not in self.result.patches:
          self.result.patches.append(SyncablePatch(patch.filename, False))

    # Iterate over the new patch list and determine whether we need to copy
    # any patch files between Chromium and Bzr. We don't attempt 3-way merging
    # of patches, as this generally either fails or produces a broken patch in
    # the case where both patches differ from the original. In this case, we
    # just mark it unresolvable

    remove_list = []

    for patch in self.result.patches:
      src_patch_cs = self.src_patches[patch.filename].checksum if patch in self.src_patches else None
      hg_patch_cs = self.hg_patches[patch.filename].checksum if patch in self.hg_patches else None
      old_patch_cs = self.old_patches[patch.filename].checksum if patch in self.old_patches else None

      if src_patch_cs == hg_patch_cs:
        # Great!
        patch.result = "resolved"
        continue

      if not hg_patch_cs:
        if not patch.active and old_patch_cs is not None:
          patch.result = "remove"
          remove_list.append(patch)
        else:
          patch.result = "use-bzr"
      elif not src_patch_cs:
        if not patch.active and old_patch_cs is not None:
          patch.result = "remove"
          remove_list.append(patch)
        else:
          patch.result = "use-hg"
      elif not old_patch_cs:
        patch.result = "conflicts"
        patch.conflicts_msg = \
            "This patch in both your Chromium checkout and revision " + \
            "control are different, and it's not possible to determine " + \
            "which one to keep. This might happen if your Chromium checkout " + \
            "was created with a revision of Oxide before r238"
      elif src_patch_cs == old_patch_cs:
        patch.result = "use-hg"
      elif hg_patch_cs == old_patch_cs:
        patch.result = "use-bzr"
      else:
        patch.result = "conflicts"
        patch.conflicts_msg = \
            "This patch in both your Chromium checkout and revision " + \
            "control are different, and it's not possible to determine " + \
            "which one to keep"

    # Remove all inactive patches that are actually going to be deleted
    for patch in remove_list:
      self.result.patches.remove(patch)

    # Determine how far back to unwind the applied patch queue
    for i in range(len(self.hg_patches)):
      hg_patch = self.hg_patches[i]
      result_patch = self.result.patches[i] if i < len(self.result.patches) else None

      if not hg_patch.applied:
        break

      if not result_patch or not result_patch.active:
        break

      if hg_patch.filename != result_patch.filename:
        break

      if (result_patch.result == "conflicts" or
          result_patch.result == "use-bzr"):
        break

      self.result.unapply_to = hg_patch.filename

    if len(self.result.conflicts) > 0:
      raise SyncError("Conflicts were found when trying to synchronize the "
                      "patch series between your Chromium checkout and "
                      "revision control")

  def do_sync(self):
    if not self.result:
      raise Exception("calculate_sync() has not been called successfully yet")

    if len(self.result.conflicts) > 0:
      raise Exception("All conflicts must be resolved before do_sync() is called")

    unapply_to = None
    if self.result.unapply_to is not None:
      unapply_to = self.hg_patches[self.result.unapply_to]
    if (unapply_to != self.hg_patches.top_patch and
        (not unapply_to or unapply_to.applied)):
      self.hg_patches.top_patch = self.result.unapply_to

    # Write the series files
    # XXX: What if any of these fail?
    self.result.patches.write_series(self.src_patches.series)
    self.result.patches.write_series(self.hg_patches.series)
    self.result.patches.write_series(os.path.join(self.hg_patches.patchdir, ".series.orig"))

    # Do copying of patches that require it and record checksums
    checksum_content = StringIO()
    for patch in self.result.patches:
      source_file = None
      dest_dir = None
      if patch.result == "use-bzr":
        source_file = os.path.join(self.src_patches.patchdir, patch.filename)
        dest_dir = self.hg_patches.patchdir
      elif patch.result == "use-hg":
        source_file = os.path.join(self.hg_patches.patchdir, patch.filename)
        dest_dir = self.src_patches.patchdir
      else:
        assert patch.result == "resolved"
        source_file = os.path.join(self.hg_patches.patchdir, patch.filename)

      checksum_content.write("%s:%s\n" %
          (patch.filename,
           GetFileChecksum(source_file)))
      if dest_dir:
        shutil.copy2(source_file, dest_dir)

    checksum_content.seek(0)
    with open(os.path.join(self.hg_patches.patchdir,
                           ".series.checksums"), "w") as fd:
      fd.write(checksum_content.read())

    # Remove files that are no longer required
    for f in self.result.files_to_remove:
      os.remove(f)

    self.refresh()

  def refresh(self):
    self.src_patches.refresh()
    self.hg_patches.refresh()
    self.old_patches.refresh()
    self.result = None
