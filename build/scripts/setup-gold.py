#!/usr/bin/python

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
from optparse import OptionParser
import os
import os.path
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), os.pardir, "python"))
from oxide_utils import CheckOutput

def which(file):
  if os.path.isabs(file):
    if os.access(file, os.F_OK | os.X_OK):
      return file
    return None

  try:
    paths = os.environ["PATH"]
  except KeyError:
    paths = os.defpath

  for path in paths.split(":"):
    full = os.path.join(path, file)
    if os.access(full, os.F_OK | os.X_OK):
      return full

def main(argv):
  parser = OptionParser()
  parser.add_option("--output", dest="output", help="Output directory")
  parser.add_option("--ld", dest="ld", help="Linker path")

  (options, args) = parser.parse_args(argv)

  output_dir = options.output
  assert output_dir != None

  ld = options.ld
  if not ld:
    ld = "ld"

  ld_basename = os.path.basename(ld)

  if os.access(os.path.join(output_dir, ld_basename),
               os.F_OK | os.X_OK):
    return

  try:
    os.makedirs(output_dir)
  except:
    pass

  gold = None

  for f in [ ld, "%s.gold" % ld ]:
    l = which(f)
    if not l:
      continue
    if CheckOutput([l, '-v']).find('gold') != -1:
      gold = l
      break

  if not gold:
    print("Cannot find a gold linker", file=sys.stderr)
    sys.exit(1)

  os.symlink(gold, os.path.join(output_dir, ld_basename))

if __name__ == "__main__":
  main(sys.argv[1:])
