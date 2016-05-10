#!/usr/bin/python
# vim:expandtab:shiftwidth=2:tabstop=2:

# Copyright (C) 2015-2016 Canonical Ltd.

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

sys.path.insert(0, os.path.join(os.path.dirname(__file__), os.pardir, "build", "python"))
from constants import (
  OXIDEDEPS_FILE,
  OXIDESRC_DIR,
  TOP_DIR,
  TOPSRC_DIR,
  TOPSRC_DIRNAME
)
from utils import (
  CheckCall,
  CheckOutput,
  IsGitRepo,
  LoadJsonFromPath
)
import subcommand

RELBRANCH_FILE = os.path.join(OXIDESRC_DIR, ".relbranch")
STATE_FILE = os.path.join(OXIDESRC_DIR, ".rebase.state")

DEV_BRANCH_PREFIX = "oxide/dev/cr"
REL_BRANCH_PREFIX = "oxide/"

GCLIENT_REVINFO_SPEC = (
  "solutions = ["
    "{ \"name\": \"%(name)s\", "
      "\"url\": \"%(url)s\", "
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

  assert "branch" in state
  assert "id" in state
  assert "merged" in state
  assert "repos" in state
  assert "rev" in state

  return state

def DoPushAndDiscardWorkingBranch(path, branch, id):
  print("Pushing changes from %s to origin/%s (%s)" % (branch, branch, path))

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
    CheckCall(["git", "push", "origin", "HEAD:%s" % branch], path)
  except:
    raise PushFailure("git push failed")

  # Return the merge revision
  rev = CheckOutput(["git", "rev-parse", "HEAD"], path).strip()

  # Check out the new rev on a detached head - this is how gclient leaves us
  CheckCall(["git", "checkout", rev], path)

  # Discard the working branch
  CheckCall(["git", "branch", "-d", working_branch], path)

  return rev

def WriteStateFile(state):
  with open(STATE_FILE, "w") as fd:
    json.dump(state, fd)

def WriteUpdatedDepsFile(deps, state):
  for path in state["repos"]:
    deps[path]["rev"] = state["repos"][path]["rev"]

  with open(OXIDEDEPS_FILE, "w") as fd:      
    json.dump(deps, fd, indent=2, sort_keys=True)

@subcommand.Command("push")
def cmd_push(options, args):
  try:
    state = LoadExistingState()
  except Exception as e:
    print("Failed to load existing state for push operation: %s" % e.message,
          file=sys.stderr)
    sys.exit(1)

  if not state["merged"]:
    print("Cannot push until the merge is completed. Please run '%s merge "
          "--continue' first" % sys.argv[0], file=sys.stderr)
    sys.exit(1)

  for path in state["repos"]:
    branch = state["repos"][path]["branch"]
    if not branch:
      print("Skipping %s - nothing to push" % path)
      continue

    try:
      rev = DoPushAndDiscardWorkingBranch(os.path.join(TOP_DIR, path),
                                          branch, state["id"])
      if rev:
        state["repos"][path]["rev"] = rev
    except PushFailure as e:
      WriteStateFile(state)
      print("** Push FAILED in %s: %s **" % (path, e.message), file=sys.stderr)
      print("** Once the problem is corrected, please rerun '%s push'**" % sys.argv[0],
            file=sys.stderr)
      sys.exit(1)

  deps = LoadJsonFromPath(OXIDEDEPS_FILE)
  WriteUpdatedDepsFile(deps, state)

  os.remove(STATE_FILE)

  print("\n** Push completed SUCCESSFULLY. Please commit changes to DEPS.oxide now **")

class MergeFailure(Exception):
  def __init__(self, msg):
    Exception.__init__(self, msg)

def SanitizeMergeArguments(options, args):
  if not options.resume:
    if len(args) != 1:
      print("Expected a merge revision", file=sys.stderr)
      sys.exit(1)
  else:
    if len(args) > 0:
      print("Expected no arguments in continue mode", file=sys.stderr)
      sys.exit(1)

    if options.master:
      print("--master is not valid with --continue", file=sys.stderr)
      sys.exit(1)

def DetermineBranchForMerge(options, rev):
  if os.path.isfile(RELBRANCH_FILE):
    with open(RELBRANCH_FILE, "r") as fd:
      return "%s%s" % (REL_BRANCH_PREFIX, fd.read().strip())

  if options.master:
    return "master"

  try:
    return ("%s%s" %
            (DEV_BRANCH_PREFIX,
             re.match(r'[\d]+\.[\d]+\.([\d]+)\.[\d]+', rev).groups()[0]))
  except:
    return None

def InitializeState(options, args, deps):
  state = {}

  if options.resume:
    try:
      state = LoadExistingState()
    except Exception as e:
      print("Failed to load existing state for merge --continue operation: %s" %
            e.message, file=sys.stderr)
      sys.exit(1)
  else:
    state["branch"] = DetermineBranchForMerge(options, args[0])
    if not state["branch"]:
      print("Unable to determine correct branch to perform the merge in",
            file=sys.stderr)
      sys.exit(1)
    state["id"] = "".join(random.choice(string.ascii_uppercase + string.digits) for i in range(8))
    state["merged"] = False
    state["repos"] = {}
    for path in deps:
      state["repos"][path] = {}
      state["repos"][path]["branch"] = None
      state["repos"][path]["rev"] = deps[path]["rev"]
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
      # Check if the new rev exists in the current HEAD
      if GitRevisionIsInBranch(rev, "HEAD", path):
        print("Nothing to do - new revision already exists in current HEAD")
        # Nothing to do
        return None

      # Check out the destination branch and make sure it's up-to-date with
      # the remote
      try:
        CheckCall(["git", "checkout", branch], path)
      except:
        raise MergeFailure(
            "Branch %s does not yet exist in repository. Please create it "
            "and ensure it's tracking a remote before trying again" % branch)

      # Sanity checks - ensure the branch we checked out is tracking the
      # correct remote
      try:
        if CheckOutput(["git", "config", "--get", "branch.%s.remote" % branch],
                       path).strip() != "origin":
          raise MergeFailure("Unexpected remote for branch")
        if CheckOutput(["git", "config", "--get", "branch.%s.merge" % branch],
                       path).strip() != "refs/heads/%s" % branch:
          raise MergeFailure("Unexpected merge ref for branch")
      except MergeFailure:
        raise
      except:
        raise MergeFailure("Branch has no remote configured")

      try:
        CheckCall(["git", "merge", "--ff-only"], path)
      except:
        raise MergeFailure(
            "Fast forward merge from remote branch failed. This might happen if "
            "your local branch contains unmerged changesets")

      # Create a working merge branch
      CheckCall(["git", "checkout", "-b", working_branch, branch], path)

    # Do the merge
    try:
      CheckCall(["git", "merge", rev, "-m", commit_string], path)
    except:
      raise MergeFailure("git merge failed")

  can_commit = False
  try:
    CheckCall(["git", "commit", "--dry-run"], path)
    can_commit = True
  except:
    pass

  if can_commit:
    CheckCall(["git", "commit", "-a", "-m", commit_string], path)

  rev = CheckOutput(["git", "rev-parse", "HEAD"], path).strip()
  return (rev, branch)

def AttemptMerge(rev, path, state):
  old_rev = state["repos"][path]["rev"]
  full_path = os.path.join(TOP_DIR, path)

  try:
    rv = DoMerge(old_rev, rev, state["branch"], full_path, state["id"])
    if rv:
      state["repos"][path]["rev"] = rv[0]
      state["repos"][path]["branch"] = rv[1]
  except MergeFailure as e:
    WriteStateFile(state)
    print("\n** Merge FAILED in %s: %s **" % (path, e.message), file=sys.stderr)
    print("** Once the problem is corrected, please rerun '%s merge --continue' **" %
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

  if not IsGitRepo(os.path.join(TOP_DIR, TOPSRC_DIRNAME)):
    print("This doesn't look like a complete checkout. This script only "
          "works on a full checkout created with fetch_oxide.py",
          file=sys.stderr)
    sys.exit(1)

  deps = LoadJsonFromPath(OXIDEDEPS_FILE)
  state = InitializeState(options, args, deps)

  rev = state["rev"]

  if not options.resume:
    CheckCall(["git", "fetch", "--all"], TOPSRC_DIR)

    try:
      CheckOutput(["git", "rev-parse", rev], TOPSRC_DIR).strip()
    except:
      print("Revision %s does not exist in repository. Have you added the "
            "upstream remotes? "
            "(tools/configure-checkout.py add-upstream-remotes)" %
            rev, file=sys.stderr)
      sys.exit(1)

    if options.master:
      # Releases are tagged in Chromium by checking out the branch
      # (creating a detached head), committing the DEPS and then tagging.
      # For merges from master, we wind back from the release tag until we
      # end up with a commit on the master branch
      while True:
        if GitRevisionIsInBranch(rev, "upstream/master", TOPSRC_DIR):
          break
        rev = CheckOutput(["git", "rev-parse", "%s^" % rev], TOPSRC_DIR).strip()
      state["rev"] = rev

  AttemptMerge(rev, TOPSRC_DIRNAME, state)

  spec = GCLIENT_REVINFO_SPEC % { "name": TOPSRC_DIRNAME,
                                  "url": deps[TOPSRC_DIRNAME]["origin"] }

  revinfo = StringIO(
      CheckOutput(["gclient", "revinfo", "--spec", spec],
                  TOP_DIR))
  for i in revinfo.readlines():
    i = i.strip()
    path = re.sub(r'([^:]*):', r'\1', i.split()[0].strip())
    if path == TOPSRC_DIRNAME:
      continue
    if path not in deps:
      continue
    rev = re.sub(r'[^@]*@(.*)', r'\1', i.split()[1].strip())

    AttemptMerge(rev, path, state)

  state["merged"] = True

  WriteStateFile(state)
  WriteUpdatedDepsFile(deps, state)

  print("\n** Merge completed SUCCESSFULLY. Please update your repository "
        "with tools/update-checkout.py, review and test your changes and then "
        "run '%s push' to push these changes to the remote repository **" %
        sys.argv[0])

def main(argv):
  return subcommand.Dispatcher.execute(argv, OptionParser())

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
