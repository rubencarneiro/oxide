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
import re
import sys

sys.dont_write_bytecode = True
os.environ["PYTHONDONTWRITEBYTECODE"] = "1"

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "build", "python"))
from oxide_utils import (
  CHROMIUMSRCDIR,
  TOPSRCDIR,
  CheckCall,
  VersionFileParser
)
import subcommand

TAR_EXCLUDE_DIRS = [
  '.bzrignore',
  'third_party/chromium/src/breakpad/src/processor/testdata',
  'third_party/chromium/src/chrome/browser/resources/tracing/tests',
  'third_party/chromium/src/chrome/common/extensions/docs',
  'third_party/chromium/src/chrome/test/data',
  'third_party/chromium/src/courgette/testdata',
  'third_party/chromium/src/native_client/src/trusted/service_runtime/testdata',
  'third_party/chromium/src/native_client/toolchain',
  'third_party/chromium/src/out',
  'third_party/chromium/src/ppapi/examples',
  'third_party/chromium/src/ppapi/native_client/tests',
  'third_party/chromium/src/third_party/angle/samples/gles2_book',
  'third_party/chromium/src/third_party/hunspell/tests',
  'third_party/chromium/src/third_party/mesa/src/src/gallium/state_trackers/d3d1x/w32api',
  'third_party/chromium/src/third_party/xdg-utils/tests',
  'third_party/chromium/src/third_party/yasm/source/patched-yasm/modules/arch/x86/tests',
  'third_party/chromium/src/third_party/yasm/source/patched-yasm/modules/dbgfmts/dwarf2/tests',
  'third_party/chromium/src/third_party/yasm/source/patched-yasm/modules/objfmts/bin/tests',
  'third_party/chromium/src/third_party/yasm/source/patched-yasm/modules/objfmts/coff/tests',
  'third_party/chromium/src/third_party/yasm/source/patched-yasm/modules/objfmts/elf/tests',
  'third_party/chromium/src/third_party/yasm/source/patched-yasm/modules/objfmts/macho/tests',
  'third_party/chromium/src/third_party/yasm/source/patched-yasm/modules/objfmts/rdf/tests',
  'third_party/chromium/src/third_party/yasm/source/patched-yasm/modules/objfmts/win32/tests',
  'third_party/chromium/src/third_party/yasm/source/patched-yasm/modules/objfmts/win64/tests',
  'third_party/chromium/src/third_party/yasm/source/patched-yasm/modules/objfmts/xdf/tests',
  'third_party/chromium/src/third_party/WebKit/LayoutTests',
  'third_party/chromium/src/third_party/WebKit/Tools/Scripts',
  'third_party/chromium/src/tools/gyp/test',
  'third_party/chromium/src/v8/test',
  'third_party/chromium/src/webkit/tools/test/reference_build',
  'client.py',
  'gclient.conf',
  'patch-tool.py',
  'release-tool.py',
]

TAR_EXCLUDE_REGEXPS = [
  r'(^|\/)\.git\/(?!(|index)$)',
  r'(^|\/)\.gitignore$',
  r'(^|\/)\.gitattributes$',
  r'(^|\/)\.hg(\/|$)',
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
@subcommand.CommandOption("-c", "--compression", dest="compression", action="store",
                          type="string", default="xz",
                          help="Specify the compression (gz, bz2 or xz)")
def cmd_make_tarball(options, args):
  """Create a tarball.

  This command will create a source tarball, which will include the
  appropriate Chromium checkout embedded and all Chromium patches applied.

  "bzr export" is not used because the tarball includes a lot of files that
  are not maintained in Bzr, and Bzr currently provides no mechanism for hooks
  to extend "export". Please do not use "bzr export", as it won't work as
  expected.
  """

  if options.compression not in ("gz", "bz2", "xz"):
    print("Invalid compression (must be \"gz\", \"bz2\" or \"xz\"", file=sys.stderr)
    sys.exit(1)

  from bzrlib.branch import Branch
  from bzrlib.workingtree import WorkingTree
  import tarfile

  # Make sure we have an up-to-date Chromium checkout
  CheckCall([sys.executable, "client.py"])

  # XXX: Qt-specific
  v = VersionFileParser(os.path.join(TOPSRCDIR, "qt", "VERSION"))
  basename = "oxide-qt"
  topsrcdir = basename

  if options.deb:
    filename = "%s_%s" % (basename, str(v))
  else:
    filename = "%s-%s" % (basename, str(v))

  topsrcdir = "%s-%s" % (basename, str(v))

  # If we're not creating a tarball from a revision with a release
  # tag, then add the revision to the filename
  branch = Branch.open(TOPSRCDIR)
  rev_id = branch.last_revision()
  tag_dict = branch.tags.get_reverse_tag_dict()
  tags = tag_dict[rev_id] if rev_id in tag_dict else []
  # XXX: Qt-specific
  if not any(re.match(r'QT_[0-9]+_[0-9]+_[0-9]+', tag) for tag in tags):
    filename = "%s~bzr%s" % (filename, branch.revision_id_to_revno(rev_id))
    topsrcdir = "%s~bzr%s" % (topsrcdir, branch.revision_id_to_revno(rev_id))

  if options.deb:
    filename = "%s.orig.tar.%s" % (filename, options.compression)
  else:
    filename = "%s.tar.%s" % (filename, options.compression)

  # Build list of files in bzr
  tree = WorkingTree.open(TOPSRCDIR)
  if tree.has_changes() and not options.force:
    print("Tree has uncommitted changes. Please commit your changes and try again", file=sys.stderr)
    sys.exit(1)

  lock = tree.lock_read()
  try:
    files = [path for (path, cls, kind, id, entry) in tree.list_files() if cls == "V"]
  finally:
    lock.unlock()

  re_excludes = [re.compile(r) for r in TAR_EXCLUDE_REGEXPS]

  def tar_filter(info):
    (root, ext) = os.path.splitext(info.name)
    if ext == ".gyp" or ext == ".gypi":
      return info
    if any(os.path.relpath(info.name, topsrcdir).startswith(r) for r in TAR_EXCLUDE_DIRS):
      return None
    if any(r.search(os.path.relpath(info.name, topsrcdir)) is not None for r in re_excludes):
      return None
    return info

  with tarfile.open(os.path.join(TOPSRCDIR, filename), "w:%s" % options.compression) as tar:
    # Add files from bzr
    for f in files:
      tar.add(f, os.path.join(topsrcdir, f), filter=tar_filter, recursive=False)

    # Add Chromium
    chromium = os.path.relpath(CHROMIUMSRCDIR, TOPSRCDIR)
    tar.add(chromium, os.path.join(topsrcdir, chromium), filter=tar_filter, recursive=True)

@subcommand.Command("release")
def cmd_tag(options, args):
  """Create a release from the current revision.

  This command will tag the current revision, and automatically increase
  the version number. Only for non-trunk branches
  """

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
  tree.commit(message="Automatically bump qt revision to %s" % str(v),
              allow_pointless=False)

def main(argv):
  return subcommand.Dispatcher.execute(argv, OptionParser())

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
