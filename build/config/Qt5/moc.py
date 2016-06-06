#!/usr/bin/python

# Copyright (C) 2016 Canonical Ltd.

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
import subprocess
import sys

def main(argv):
  parser = OptionParser(usage="usage: %prog [options] input output")
  parser.add_option("-m", dest="moc")

  (options, args) = parser.parse_args(argv)

  if len(args) != 2:
    print("Invalid number of arguments", file=sys.stderr)
    parser.print_usage(file=sys.stderr)
    return 1

  if not options.moc:
    print("No moc executable specified", file=sys.stderr)
    return 1

  moc = os.path.abspath(options.moc)
  if not os.access(moc, os.X_OK):
    print("moc is not executable", file=sys.stderr)
    return 1

  subprocess.check_call([moc, "-o", args[1], args[0]])

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
