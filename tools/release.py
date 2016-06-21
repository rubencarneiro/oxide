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
  ScopedTmpdir,
  VersionFileParser
)
import subcommand

# Contents of paths matching any of these entries will be excluded from the
#  tarball, with the exception of build files with extensions matching any
#  of those in TAR_INCLUDE_EXTENSIONS
TAR_EXCLUDE_CONTENTS_PATHS = [
  'chrome/test/data',
  'v8/test'
]

# Paths matching any of these entries will be excluded from the tarball
TAR_EXCLUDE_PATHS = [
  'breakpad/src/processor/testdata',
  'chrome/common/extensions/docs',
  'courgette/testdata',
  'native_client/src/trusted/service_runtime/testdata',
  'native_client/toolchain',
  'out',
  'oxide/.relbranch',
  'ppapi/native_client/tests',
  'third_party/hunspell/tests',
  'third_party/mesa/src/src/gallium/state_trackers/d3d1x/w32api',
  'third_party/sqlite/src/test',
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
]

# Paths matching any of these expressions will be excluded from the tarball
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

TAR_INCLUDE_EXTENSIONS = [
  '.gn',
  '.gni',
  '.grd',
  '.gyp',
  '.gypi',
  '.isolate'
]

class OptionParser(optparse.OptionParser):
  def __init__(self):
    optparse.OptionParser.__init__(self)

    self.description = """
Tool for handling release tasks.
"""
    self.add_option("--platform", dest="platform",
                    help="The Oxide platform to create a release for",
                    default="qt")
    self.add_option("-v", "--verbose", action="store_true",
                    help="Verbose output")

@subcommand.Command("make-tarball")
@subcommand.CommandOption("--deb", dest="deb", action="store_true",
                          help="Create a tarball suitable for Ubuntu / Debian "
                               "packaging")
@subcommand.CommandOption("-f", "--force", dest="force", action="store_true",
                          help="Create a tarball even if the tree has "
                               "uncommitted changes")
def cmd_make_tarball(options, args):
  """Create a tarball.

  This command will create a source tarball, which will include the
  appropriate Chromium checkout embedded and all Chromium patches applied.
  """

  import tarfile

  if options.verbose:
    print("Updating checkout...")

  # Make sure we have an up-to-date Chromium checkout
  #CheckCall(["tools/update-checkout.py"], OXIDESRC_DIR)

  platform = options.platform

  v = VersionFileParser(os.path.join(OXIDESRC_DIR, platform, "VERSION"))
  basename = "oxide-%s" % platform
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
    if re.match(r'%s_[0-9]+_[0-9]+_[0-9]+' % platform.upper(), tag):
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
    if (any(os.path.relpath(info.name, topsrcdir).startswith(r) for r in TAR_EXCLUDE_PATHS) or
        any(r.search(os.path.relpath(info.name, topsrcdir)) is not None for r in re_excludes)):
      print_skipped_file(info.name)
      return None
    if ext in TAR_INCLUDE_EXTENSIONS or info.isdir():
      print_added_file(info.name)
      return info
    if any(os.path.relpath(os.path.dirname(info.name), topsrcdir).startswith(r) for r in TAR_EXCLUDE_CONTENTS_PATHS):
      print_skipped_file(info.name)
      return None
    print_added_file(info.name)
    return info

  output_path = os.path.join(TOP_DIR, filename)

  with tarfile.open(output_path, "w") as tar:
    for name in os.listdir(TOPSRC_DIR):
      tar.add(os.path.join(TOPSRC_DIR, name), os.path.join(topsrcdir, name),
              filter=tar_filter, recursive=True)

    with ScopedTmpdir() as tmpdir:
      angle = os.path.join("third_party", "angle")
      angle_src = os.path.join(angle, "src")
      commit_id_h = os.path.join(angle_src, "commit.h")
      if options.verbose:
        print("Generating %s" % commit_id_h)
      os.makedirs(os.path.join(tmpdir, angle_src))
      CheckCall([sys.executable,
                 os.path.join(TOPSRC_DIR, angle_src, "commit_id.py"),
                 "gen", os.path.join(TOPSRC_DIR, angle),
                 os.path.join(tmpdir, commit_id_h)],
                TOPSRC_DIR)
      for name in os.listdir(tmpdir):
        tar.add(os.path.join(tmpdir, name), os.path.join(topsrcdir, name),
                recursive=True)

  if options.verbose:
    print("Compressing tarball")
  CheckCall(["xz", "-9", output_path], TOP_DIR)

@subcommand.CommandOption("--no-push", dest="no_push", action="store_true",
                          help="Don't push changes to the remote repository")
@subcommand.Command("release")
def cmd_tag(options, args):
  """Create a release from the current revision.

  This command will tag the current revision, and automatically increase
  the version number. Only for non-trunk branches
  """

  if len(CheckOutput(["git", "status", "--porcelain"], OXIDESRC_DIR)) > 0:
    print("Cannot tag release - this branch has uncommitted changes",
          file=sys.stderr)
    return 1

  if not os.path.isfile(os.path.join(OXIDESRC_DIR, ".relbranch")):
    print("This command is only valid for release branches", file=sys.stderr)
    return 1

  with open(os.path.join(OXIDESRC_DIR, ".relbranch"), "r") as fd:
    relbranch = fd.read().strip()
    if options.verbose:
      print("Detected release branch '%s'" % relbranch)

  try:
    local_branch = CheckOutput(["git", "rev-parse", "--symbolic-full-name", "--abbrev-ref", "@"],
                               OXIDESRC_DIR).strip()
    if options.verbose:
      print("Detected local branch '%s'" % local_branch)
  except:
    print("Cannot tag a release on a branch that is in 'detached HEAD' state",
          file=sys.stderr)
    return 1

  if local_branch != relbranch:
    print("Cannot tag release - unexpected branch '%s', (expected 'refs/heads/%s')\n" %
          (local_branch, relbranch), file=sys.stderr)
    return 1

  remote_branch = None
  try:
    remote_branch = CheckOutput(["git", "rev-parse", "--symbolic-full-name", "--abbrev-ref", "@{u}"],
                                OXIDESRC_DIR).strip()
    if options.verbose:
      print("Detected remote branch '%s'" % remote_branch)
  except:
    print("Cannot tag release - no remote tracking branch for the current branch",
          file=sys.stderr)
    return 1

  if (remote_branch and
      CheckOutput(["git", "rev-parse", remote_branch], OXIDESRC_DIR).strip() !=
      CheckOutput(["git", "rev-parse", "@"], OXIDESRC_DIR).strip()):
    print("Cannot tag release - current branch is behind remote tracking branch",
          file=sys.stderr)
    return 1

  platform = options.platform

  v = VersionFileParser(os.path.join(OXIDESRC_DIR, platform, "VERSION"))
  tag = "%s_%s_%s_%s" % (platform.upper(), v.major, v.minor, v.patch)

  if options.verbose:
    print("Tagging release with the tag '%s'" % tag)

  try:
    CheckCall(["git", "tag", tag], OXIDESRC_DIR)
  except:
    print("Failed to create tag '%s'\n" % tag, file=sys.stderr)
    return 1

  v.patch = str(int(v.patch) + 1)

  if options.verbose:
    print("Bumping version to '%s'" % str(v))

  v.update()

  CheckCall(["git", "commit", "-am", "Bump %s revision to %s" % (platform, str(v))],
            OXIDESRC_DIR)

  if options.no_push:
    return 0

  r = re.match(r'^([^\/]+)\/(.+)', remote_branch)

  if options.verbose:
    print("Pushing changes to remote repository")

  try:
    CheckCall(["git", "push", "--tags", r.groups()[0],
               "%s:%s" % (local_branch, r.groups()[1])], OXIDESRC_DIR)
  except:
    print("Failed to push to remote repository. This is probably caused by a "
          "race with another push", file=sys.stderr)
    print("Rolling back local changes", file=sys.stderr)
    CheckCall(["git", "reset", "--hard", remote_branch], OXIDESRC_DIR)
    CheckCall(["git", "tag", "-d", tag], OXIDESRC_DIR)

def main(argv):
  return subcommand.Dispatcher.execute(argv, OptionParser())

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
