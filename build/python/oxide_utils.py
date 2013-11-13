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

import base64
import hashlib
import os
import os.path
import shutil
from subprocess import Popen, CalledProcessError, PIPE

TOPSRCDIR = os.path.abspath(os.path.join(__file__, os.pardir, os.pardir, os.pardir))
CHROMIUMDIR = os.path.join(TOPSRCDIR, "chromium")
CHROMIUMSRCDIR = os.path.join(CHROMIUMDIR, "src")

def GetFileChecksum(file):
  """Return a SHA256 hash from the contents of the specified filename"""
  h = hashlib.sha256()
  with open(file, "r") as fd:
    h.update(fd.read())
  return base64.b16encode(h.digest())

def CheckCall(args, cwd=None):
  p = Popen(args, cwd=cwd)
  r = p.wait()
  if r is not 0: raise CalledProcessError(r, args)

def CheckOutput(args, cwd=None):
  e = os.environ
  e['LANG'] = 'C'
  p = Popen(args, cwd=cwd, stdout=PIPE, env=e)
  r = p.wait()
  if r is not 0: raise CalledProcessError(r, args)
  return p.stdout.read()
