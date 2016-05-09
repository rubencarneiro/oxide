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
import optparse
import os.path
import re
from StringIO import StringIO
import sys

sys.dont_write_bytecode = True
os.environ["PYTHONDONTWRITEBYTECODE"] = "1"

sys.path.insert(0, os.path.join(os.path.dirname(__file__), os.pardir, "build", "python"))
from constants import (
  OXIDESRC_DIR,
  TOPSRC_DIR,
  TOP_DIR
)
from utils import (
  CheckCall,
  CheckOutput,
  VersionFileParser
)
import subcommand

TAR_EXCLUDE_PATHS = [
  'breakpad/src/processor/testdata',
  'chrome/browser/resources/tracing/tests',
  'chrome/common/extensions/docs',
  'courgette/testdata',
  'native_client/src/trusted/service_runtime/testdata',
  'native_client/toolchain',
  'oxide/.relbranch',
  'out',
  'ppapi/examples',
  'ppapi/native_client/tests',
  'third_party/angle/samples/gles2_book',
  'third_party/hunspell/tests',
  'third_party/mesa/src/src/gallium/state_trackers/d3d1x/w32api',
  'third_party/xdg-utils/tests',
  'third_party/yasm/source/patched-yasm/modules/arch/x86/tests',
  'third_party/yasm/source/patched-yasm/modules/dbgfmts/dwarf2/tests',
  'third_party/yasm/source/patched-yasm/modules/objfmts/bin/tests',
  'third_party/yasm/source/patched-yasm/modules/objfmts/coff/tests',
  'third_party/yasm/source/patched-yasm/modules/objfmts/elf/tests',
  'third_party/yasm/source/patched-yasm/modules/objfmts/macho/tests',
  'third_party/yasm/source/patched-yasm/modules/objfmts/rdf/tests',
  'third_party/yasm/source/patched-yasm/modules/objfmts/win32/tests',
  'third_party/yasm/source/patched-yasm/modules/objfmts/win64/tests',
  'third_party/yasm/source/patched-yasm/modules/objfmts/xdf/tests',
  'third_party/WebKit/LayoutTests',
  'third_party/WebKit/Tools/Scripts',
  'tools/gyp/test',
  'v8/test',
  'webkit/tools/test/reference_build',
]

TAR_EXCLUDE_REGEXPS = [
  r'^objdir.*',
  r'(^|\/)\.git(\/|$)',
  r'(^|\/)\.gitignore$',
  r'(^|\/)\.gitattributes$',
  r'(^|\/)\.svn(\/|$)',
  r'\.mk$',
  r'\.o$',
  r'\.so(|\..*)$',
  r'\.pyc$',
]

class OptionParser(optparse.OptionParser):
  def __init__(self):
    optparse.OptionParser.__init__(self)

    self.description = """
Tool for handling release tasks.
"""

@subcommand.Command("make-tarball")
@subcommand.CommandOption("--deb", dest="deb", action="store_true",
                          help="Create a tarball suitable for Ubuntu / Debian "
                               "packaging")
@subcommand.CommandOption("-f", "--force", dest="force", action="store_true",
                          help="Create a tarball even if the tree has "
                               "uncommitted changes")
@subcommand.CommandOption("-v", "--verbose", action="store_true",
                          help="Enable logging")
def cmd_make_tarball(options, args):
  """Create a tarball.

  This command will create a source tarball, which will include the
  appropriate Chromium checkout embedded and all Chromium patches applied.
  """

  import tarfile

  if options.verbose:
    print("Updating checkout...")

  # Make sure we have an up-to-date Chromium checkout
  CheckCall(["tools/update-checkout.py"], OXIDESRC_DIR)

  # XXX: Qt-specific
  v = VersionFileParser(os.path.join(OXIDESRC_DIR, "qt", "VERSION"))
  basename = "oxide-qt"
  topsrcdir = basename

  if options.deb:
    filename = "%s_%s" % (basename, str(v))
  else:
    filename = "%s-%s" % (basename, str(v))

  topsrcdir = "%s-%s" % (basename, str(v))

  is_release = False
  tags = StringIO(CheckOutput(["git", "tag", "--points-at", "HEAD"], OXIDESRC_DIR))
  for tag in tags.readlines():
    tag = tag.strip()
    # XXX: Qt-specific
    if re.match(r'QT_[0-9]+_[0-9]+_[0-9]+', tag):
      is_release = True
      break

  if not is_release:
    timestamp = CheckOutput(["git", "log", "-n1", "--pretty=format:%ct"],
                            OXIDESRC_DIR).strip()
    commit_hash = CheckOutput(["git", "log", "-n1", "--pretty=format:%h"],
                              OXIDESRC_DIR).strip()
    filename = "%s~git%s.%s" % (filename, timestamp, commit_hash)
    topsrcdir = "%s~git%s.%s" % (topsrcdir, timestamp, commit_hash)

  if options.deb:
    filename = "%s.orig.tar" % filename
  else:
    filename = "%s.tar" % filename

  re_excludes = [re.compile(r) for r in TAR_EXCLUDE_REGEXPS]

  def print_added_file(name):
    if not options.verbose:
      return
    print("Adding %s" % name)

  def print_skipped_file(name):
    if not options.verbose:
      return
    print("Skipping %s" % name)

  def tar_filter(info):
    (root, ext) = os.path.splitext(info.name)
    if ext == ".gyp" or ext == ".gypi":
      print_added_file(info.name)
      return info
    if any(os.path.relpath(info.name, topsrcdir).startswith(r) for r in TAR_EXCLUDE_PATHS):
      print_skipped_file(info.name)
      return None
    if any(r.search(os.path.relpath(info.name, topsrcdir)) is not None for r in re_excludes):
      print_skipped_file(info.name)
      return None
    print_added_file(info.name)
    return info

  output_path = os.path.join(TOP_DIR, filename)

  with tarfile.open(output_path, "w") as tar:
    for name in os.listdir(TOPSRC_DIR):
      tar.add(os.path.join(TOPSRC_DIR, name), os.path.join(topsrcdir, name),
              filter=tar_filter, recursive=True)

  if options.verbose:
    print("Compressing tarball")
  CheckCall(["xz", "-9", output_path], TOP_DIR)

@subcommand.Command("release")
def cmd_tag(options, args):
  """Create a release from the current revision.

  This command will tag the current revision, and automatically increase
  the version number. Only for non-trunk branches
  """

  print("This command needs to be updated for GIT!", file=sys.stderr)
  sys.exit(1)

  from bzrlib.branch import Branch
  from bzrlib.errors import (
    NoSuchTag,
    TagAlreadyExists
  )
  from bzrlib.workingtree import WorkingTree

  # XXX: Qt-specific
  v = VersionFileParser(os.path.join(TOPSRCDIR, "qt", "VERSION"))

  branch = Branch.open(TOPSRCDIR)
  lock = branch.lock_write()

  try:
    # XXX: Qt-specific
    tag = "QT_%s_%s_%s" % (v.major, v.minor, v.patch)
    rev_id = branch.last_revision()
    try:
      existing_tag = branch.tags.lookup_tag(tag)
    except NoSuchTag:
      existing_tag = None
    if existing_tag is not None:
      raise TagAlreadyExists(tag)

    branch.tags.set_tag(tag, rev_id)
  finally:
    lock.unlock()

  v.patch = str(int(v.patch) + 1)
  v.update()

  tree = WorkingTree.open(TOPSRCDIR)
  # XXX: Qt-specific
  tree.commit(message="Bump qt revision to %s" % str(v),
              allow_pointless=False)

def main(argv):
  return subcommand.Dispatcher.execute(argv, OptionParser())

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
