#!/usr/bin/python
# vim:expandtab:shiftwidth=2:tabstop=2:

# Copyright (C) 2015 Canonical Ltd.

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
from StringIO import StringIO
import json
import os
import os.path
import random
import re
import string
import sys

sys.dont_write_bytecode = True
os.environ["PYTHONDONTWRITEBYTECODE"] = "1"

sys.path.insert(0, os.path.join(os.path.dirname(__file__), os.pardir, "python"))
from utils import (
  CheckCall,
  CheckOutput,
  CHECKOUT_CONFIG,
  CHROMIUMDIR,
  CHROMIUMSRCDIR,
  CHROMIUMSRCDIR_REL,
  DEPOTTOOLSDIR,
  LoadJsonFromPath,
  TOPSRCDIR
)
import subcommand

STATE_FILE = os.path.join(TOPSRCDIR, ".rebase.state")

GCLIENT_REVINFO_SPEC = (
  "solutions = ["
    "{ \"name\": \"src\", "
      "\"url\": \"%s\", "
      "\"deps_file\": \"DEPS\", "
      "\"managed\": False, "
    "} "
  "]"
)

class Options(OptionParser):
  def __init__(self):
    OptionParser.__init__(self)

    self.description = """
A tool for updating the Chromium version in Oxide
"""

@subcommand.Command("abort")
def cmd_abort(options, args):
  raise NotImplemented

class PushFailure(Exception):
  def __init__(self, msg):
    Exception.__init__(self, msg)

def LoadExistingState():
  state = LoadJsonFromPath(STATE_FILE)

  assert "config" in state
  assert "id" in state
  assert "master" in state
  assert "merged" in state
  assert "rev" in state

  return state

def DoPush(path, branch, id):
  print("Pushing changes from %s to origin/%s" % (path, branch))

  working_branch = "rebase-%s" % id

  # Checkout the working branch, if there is one
  try:
    CheckCall(["git", "checkout", working_branch], path, True)
  except:
    print("Nothing to push")
    return None

  # Pull from the remote branch
  try:
    CheckCall(["git", "pull", "--commit", "origin", branch], path)
  except:
    raise PushFailure("git pull failed")

  # Push the merge result
  try:
    CheckCall(["git", "push", "origin", branch], path)
  except:
    raise PushFailure("git push failed")

  try:
    CheckCall(["git", "fetch", "upstream"], path)
  except:
    raise PushFailure("git fetch failed")

  # Capture the merge revision
  rev = CheckOutput(["git", "rev-parse", "HEAD"], path).strip()

  # Fast forward the local branch
  CheckCall(["git", "checkout", branch], path)
  try:
    CheckCall(["git", "merge", "--ff-only"], path)
  except:
    print("Failed to fast-forward the local branch. This might be caused by unmerged "
          "changesets", file=sys.stderr)

  # Check out the new rev on a detached head
  CheckCall(["git", "checkout", rev], path)

  # Discard the working branch
  CheckCall(["git", "branch", "-d", working_branch], path)

  return rev

@subcommand.Command("push")
def cmd_push(options, args):
  try:
    state = LoadExistingState()
  except Exception as e:
    print("Failed to load existing state for push operation: %s" % e.message,
          file=sys.stderr)
    sys.exit(1)

  if not state["merged"]:
    print("Cannot push until the merge is completed. Please run %s merge "
          "--continue first" % sys.argv[0], file=sys.stderr)
    sys.exit(1)

  for path in state["config"]:
    try:
      rev = DoPush(os.path.join(CHROMIUMDIR, path), "master", state["id"])
      if rev:
        state["config"][path]["rev"] = rev
    except PushFailure as e:
      with open(STATE_FILE, "w") as fd:
        json.dump(state, fd)
      print("** Push FAILED in %s: %s **" % (path, e.message), file=sys.stderr)
      print("** Once the problem is corrected, please rerun %s push**" % sys.argv[0],
            file=sys.stderr)
      sys.exit(1)

  with open(CHECKOUT_CONFIG, "w") as fd:
    json.dump(state["config"], fd, indent=2)

  os.remove(STATE_FILE)

  print("\n** Push completed SUCCESSFULLY. Please commit changes to checkout.conf now **")

class MergeFailure(Exception):
  def __init__(self, msg):
    Exception.__init__(self, msg)

def SanitizeMergeArguments(options, args):
  if not options.resume:
    if len(args) != 1:
      print("Expected a merge revision", file=sys.stderr)
      sys.exit(1)

    if not options.master:
      print("Only merges from master are supported right now",
            file=sys.stderr)
      sys.exit(1)
  else:
    if len(args) > 0:
      print("Expected no arguments in continue mode", file=sys.stderr)
      sys.exit(1)

    if options.master:
      print("--master is not valid with --continue", file=sys.stderr)
      sys.exit(1)

def InitializeState(options, args, config):
  state = {}

  if options.resume:
    try:
      state = LoadExistingState()
    except Exception as e:
      print("Failed to load existing state for merge --continue operation: %s" %
            e.message, file=sys.stderr)
      sys.exit(1)
  else:
    state["config"] = config
    state["id"] = "".join(random.choice(string.ascii_uppercase + string.digits) for i in range(8))
    state["master"] = True if options.master else False
    state["merged"] = False
    state["rev"] = args[0]

  return state

def GitRevisionIsInBranch(rev, branch, path):
  args = ["git", "branch"]
  if branch.startswith("upstream/") or branch.startswith("origin/"):
    args.append("-r")
  args.append(branch)
  args.append("--contains")
  args.append(rev)
  return CheckOutput(args, path).strip() != ""

def DoMerge(old_rev, rev, branch, path, id):
  print("\nBeginning merge of revision %s in to branch %s (%s)" % (rev, branch, path))

  working_branch = "rebase-%s" % id
  commit_string = "Merge upstream %s in to %s" % (rev, branch)

  # If there is an unfinished merge, we skip straight to commit
  if not os.path.isfile(os.path.join(path, ".git", "MERGE_HEAD")):
    # Check if the working branch already exists
    in_working_branch = False
    try:
      CheckCall(["git", "rev-parse", "--verify", working_branch], path, True)
      in_working_branch = True
    except:
      pass

    # If the working branch doesn't exist, create one
    if not in_working_branch:
      # Sanity check - current HEAD should be what's in checkout.conf
      if CheckOutput(["git", "rev-parse", "HEAD"], path).strip() != old_rev:
        raise MergeFailure("Current HEAD does not match the expected revision "
                           "in checkout.conf. Do you have local changes?")

      assert GitRevisionIsInBranch(old_rev, branch, path)

      # Check if the new rev exists in the current HEAD
      if GitRevisionIsInBranch(rev, "HEAD", path):
        print("Nothing to do - new revision already exists in current HEAD")
        # Nothing to do
        return

      # Check out the destination branch and make sure it's up-to-date with
      # the remote
      CheckCall(["git", "checkout", branch], path)
      try:
        CheckCall(["git", "merge", "--ff-only"], path)
      except:
        # We haven't even created the working branch, let alone started the
        # merge. In this case, we go back to the original state
        CheckCall(["git", "checkout", old_rev], path)
        raise MergeFailure(
            "Fast forward merge from remote branch failed. This might happen if "
            "your local branch contains unmerged changesets")

      # Another sanity check
      if GitRevisionIsInBranch(rev, "HEAD", path):
        print("The new revision already does exist in %s, but not in the "
              "current HEAD. This probably means that somebody did a manual "
              "merge and didn't update checkout.conf. Tut tut")

      # Create a working merge branch
      CheckCall(["git", "checkout", "-b", working_branch, branch], path)

    # Do the merge
    try:
      CheckCall(["git", "merge", rev, "-m", commit_string], path)
    except:
      raise MergeFailure("git merge failed")

  # XXX: Check that the merge is ready to commit

  can_commit = False
  try:
    CheckCall(["git", "commit", "--dry-run"], path)
    can_commit = True
  except:
    pass

  if can_commit:
    CheckCall(["git", "commit", "-a", "-m", commit_string], path)

def AttemptMerge(rev, branch, path, state):
  full_path = os.path.join(CHROMIUMDIR, path)
  old_rev = state["config"][path]["rev"]
  try:
    DoMerge(old_rev, rev, branch, full_path, state["id"])
  except MergeFailure as e:
    with open(STATE_FILE, "w") as fd:
      json.dump(state, fd)
    print("\n** Merge FAILED in %s: %s **" % (path, e.message), file=sys.stderr)
    print("** Once the problem is corrected, please rerun %s merge --continue **" %
          sys.argv[0], file=sys.stderr)
    sys.exit(1)

@subcommand.Command("merge")
@subcommand.CommandOption(
    "--master", dest="master", action="store_true",
    help="Perform the merge on the master branch. In this mode, we always "
         "merge from Chromium master")
@subcommand.CommandOption("--continue", dest="resume", action="store_true",
                          help="Resume a previously started merge")
def cmd_merge(options, args):
  SanitizeMergeArguments(options, args)

  config = LoadJsonFromPath(CHECKOUT_CONFIG)
  state = InitializeState(options, args, config)
  rev = state["rev"]

  if not options.resume:
    CheckCall(["git", "fetch", "--all"], CHROMIUMSRCDIR)

    try:
      rev = CheckOutput(["git", "rev-parse", rev], CHROMIUMSRCDIR).strip()
    except:
      print("Revision %s does not exist in repository. Have you added the "
            "upstream remotes? (init-checkout.py --add-upstream-remotes)" %
            rev, file=sys.stderr)
      sys.exit(1)

    if options.master:
      # Releases are tagged in Chromium by checking out the branch
      # (creating a detached head), committing the DEPS and then tagging.
      # For merges from master, we wind back from the release tag until we
      # end up with a commit on the master branch
      while True:
        if GitRevisionIsInBranch(rev, "upstream/master", CHROMIUMSRCDIR):
          break
        rev = CheckOutput(["git", "rev-parse", "%s^" % rev], CHROMIUMSRCDIR).strip()

  # XXX: For non-master merges on oxide trunk, we will have a branch name
  #  here of the form "oxide/dev/<CHROMIUM_BRANCH>". We can get the branch
  #  from the Chromium tag specified by the caller. For merges on oxide
  #  release branches, we'll have a branch of the form "oxide/<OXIDE_BRANCH>".
  #  We could probably get this number from the source tree.
  AttemptMerge(rev, "master", CHROMIUMSRCDIR_REL, state)

  spec = GCLIENT_REVINFO_SPEC % config[CHROMIUMSRCDIR_REL]["custom_remote"]

  revinfo = StringIO(
      CheckOutput([sys.executable, os.path.join(DEPOTTOOLSDIR, "gclient.py"),
                   "revinfo", "--spec", spec], CHROMIUMDIR))
  for i in revinfo.readlines():
    i = i.strip()
    path = re.sub(r'([^:]*):', r'\1', i.split()[0].strip())
    if path == CHROMIUMSRCDIR_REL:
      continue
    if path not in config:
      continue
    rev = re.sub(r'[^@]*@(.*)', r'\1', i.split()[1].strip())

    AttemptMerge(rev, "master", path, state)

  state["merged"] = True

  with open(STATE_FILE, "w") as fd:
    json.dump(state, fd)

  print("\n** Merge completed SUCCESSFULLY. Please review changes and then run "
        "%s push to push these changes to the remote repository **" % sys.argv[0])

def main(argv):
  return subcommand.Dispatcher.execute(argv, OptionParser())

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
