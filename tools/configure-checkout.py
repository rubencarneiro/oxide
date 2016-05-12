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
from optparse import OptionParser
import os
import re
from StringIO import StringIO
import sys
from urlparse import urlsplit

sys.dont_write_bytecode = True
os.environ["PYTHONDONTWRITEBYTECODE"] = "1"

sys.path.insert(0, os.path.join(os.path.dirname(__file__), os.pardir, "build", "python"))
from constants import (
  OXIDEDEPS_FILE,
  OXIDESRC_DIR,
  TOP_DIR,
  TOPSRC_DIRNAME
)
import subcommand
from utils import (
  CheckCall,
  CheckOutput,
  GetGitConfig,
  IsGitRepo,
  LoadJsonFromPath
)

@subcommand.Command("add-upstream-remotes")
@subcommand.CommandOption("--no-fetch", action="store_true",
                          help="Don't run 'git fetch upstream' after adding "
                               "the upstream remotes")
def cmd_add_upstream_remotes(options, args):
  """Add upstream remotes to all repositories listed in DEPS.oxide with an upstream URL.
  """

  deps = LoadJsonFromPath(OXIDEDEPS_FILE)
  for dep in deps:
    if "upstream" not in deps[dep]:
      continue
    path = os.path.join(TOP_DIR, dep)
    if not IsGitRepo(path):
      print("Path '%s' is not a GIT repository. Is this a complete checkout?"
            % path, file=sys.stderr)
      sys.exit(1)
    try:
      CheckCall(["git", "remote", "show", "upstream"], path, True)
      print("Repository '%s' already has a remote called 'upstream'" % path)
    except:
      print("Adding 'upstream' remote to repository '%s'" % path)
      CheckCall(["git", "remote", "add", "upstream",
                 deps[dep]["upstream"]], path)
      extra_refs = [ "refs/branch-heads/*" ]
      if dep == TOPSRC_DIRNAME:
        extra_refs.append("refs/tags/*")
      for r in extra_refs:
        CheckCall(["git", "config", "--add", "remote.upstream.fetch",
                   "+%s:%s" % (r, r)], path)
      if not options.no_fetch:
        CheckCall(["git", "fetch", "upstream"], path)

def AddSshPushUrl(path, user_id):
  origin = CheckOutput(["git", "config", "remote.origin.url"], path).strip()
  u = urlsplit(origin)
  if u.scheme not in ("git", "https"):
    print("Skipping checkout '%s' with url '%s', which doesn't have an "
          "expected scheme" % (path, origin))
    return
  if u.netloc != "git.launchpad.net":
    print("Skipping checkout '%s' with unexpected host '%s'" %
          (path, u.netloc))
    return
  u = u._replace(scheme="git+ssh")
  u = u._replace(netloc="%s@%s" % (user_id, u.netloc))
  print("Adding push URL '%s' to origin for '%s'" % (u.geturl(), path))
  CheckCall(["git", "remote", "set-url", "--push", "origin", u.geturl()], path)

@subcommand.Command("add-ssh-push-urls", usage_more=" [path1] [path2] .. [pathN]")
@subcommand.CommandOption("-a", "--all", action="store_true",
                          help="Add SSH push URLs to all branches listed in "
                               "DEPS.oxide")
@subcommand.CommandOption("-u", "--user-id", help="Your Launchpad user ID")
def cmd_add_ssh_push_urls(options, args):
  """Configure repositories listed in DEPS.oxide with git+ssh:// push URLs.

  In a normal checkout, the origin for repositories listed in DEPS.oxide is a
  https:// URL. As Launchpad only supports pushing via SSH, this command adds
  a git+ssh:// push URL to these repositories. You can do this when creating
  a checkout by passing --user-id to fetch_oxide
  """

  if options.all and len(args) > 0:
    print("Mixing --all with specific paths does not make sense",
          file=sys.stderr)
    sys.exit(1)

  if not options.user_id:
    print("Missing --user-id option", file=sys.stderr)
    sys.exit(1)

  deps = LoadJsonFromPath(OXIDEDEPS_FILE)

  paths = []
  if options.all:
    paths = [ os.path.join(TOP_DIR, path) for path in deps ]
    paths.append(OXIDESRC_DIR)
  else:
    if len(args) == 0:
      print("Missing paths to git checkouts", file=sys.stderr)
      sys.exit(1)
    paths = [ os.path.abspath(arg) for arg in args ]

  for path in paths:
    if not IsGitRepo(path):
      print("Path '%s' is not a GIT repository" % path, file=sys.stderr)
      sys.exit(1)

    relpath = os.path.relpath(path, TOP_DIR)
    if relpath not in deps and path != OXIDESRC_DIR:
      print("Path '%s' does not appear in DEPS.oxide" % path, file=sys.stderr)
      sys.exit(1)

    AddSshPushUrl(path, options.user_id)

def DissociateRepo(path):
  print("Dissociating repo at %s" % path)
  CheckCall(["git", "repack", "-a"], path)
  try:
    os.remove(os.path.join(path, ".git", "objects", "info", "alternatives"))
  except:
    pass

@subcommand.Command("dissociate")
def cmd_dissociate(options, args):
  """Dissociate a checkout from its local mirror"""

  cache_dir = GetGitConfig("oxide.cacheDir", OXIDESRC_DIR)
  if not cache_dir:
    print("This checkout was not cloned from a local mirror")
    sys.exit(0)

  cache_mode = GetGitConfig("oxide.cacheMode", OXIDESRC_DIR)
  if cache_mode != "reference":
    print("Cannot dissociate checkouts created with --cache-mode=full",
          file=sys.stderr)
    sys.exit(1)

  DissociateRepo(OXIDESRC_DIR)

  revinfo = StringIO(CheckOutput(["gclient", "revinfo"], TOP_DIR))
  for i in revinfo.readlines():
    i = i.strip().split()
    if i[1].strip() == "None":
      continue
    path = re.sub(r'([^:]*):', r'\1', i[0].strip())
    DissociateRepo(os.path.join(TOP_DIR, path))

  CheckCall(["git", "config", "--unset", "oxide.cacheDir"], OXIDESRC_DIR)
  CheckCall(["git", "config", "--unset", "oxide.cacheMode"], OXIDESRC_DIR)

def main(argv):
  return subcommand.Dispatcher.execute(argv, OptionParser())

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
