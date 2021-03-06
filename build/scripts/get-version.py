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
import os
import os.path
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), os.pardir, "python"))
from constants import OXIDESRC_DIR
from utils import VersionFileParser

def main(args):
  if len(args) < 1:
    print("Usage: get-version.py <port>", file=sys.stderr)
    sys.exit(1)

  v = VersionFileParser(os.path.join(OXIDESRC_DIR, args[0], "VERSION"))

  print(v)

if __name__ == "__main__":
  main(sys.argv[1:])
