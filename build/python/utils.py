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
import json
import os
import os.path
import re
import shutil
from subprocess import Popen, CalledProcessError, PIPE
import tempfile

class VersionFileParserError(Exception):
  pass

class VersionFileParser(object):
  def __init__(self, file):
    self._dirty = False
    self._v = [None for i in range(3)]

    self._filename = file

    with open(self._filename, "r") as fd:
      r = re.compile(r'[^=]*=([0-9]+)')
      for line in fd.readlines():
        try:
          if line.startswith("MAJOR="):
            self._v[0] = r.match(line.strip()).group(1)
          elif line.startswith("MINOR="):
            self._v[1] = r.match(line.strip()).group(1)
          elif line.startswith("PATCH="):
            self._v[2] = r.match(line.strip()).group(1)
          else:
            raise VersionFileParserError("Unrecognized line '%s'" % line.strip())
        except VersionFileParserError:
          raise
        except AttributeError:
          raise VersionFileParserError("Invalid version number in line '%s'" % line.strip())

    if any(i == None for i in self._v):
      raise VersionFileParserError("Incomplete version number")

  def __str__(self):
    return "%s.%s.%s" % (self._v[0], self._v[1], self._v[2])

  def update(self):
    if not self._dirty:
      return

    with open(self._filename, "w") as fd:
      fd.write("MAJOR=%s\n" % self._v[0])
      fd.write("MINOR=%s\n" % self._v[1])
      fd.write("PATCH=%s" % self._v[2])

    self._dirty = False

  @property
  def major(self):
    return self._v[0]

  @major.setter
  def major(self, major):
    self._v[0] = major
    self._dirty = True

  @property
  def minor(self):
    return self._v[1]

  @minor.setter
  def minor(self, minor):
    self._v[1] = minor
    self._dirty = True

  @property
  def patch(self):
    return self._v[2]

  @patch.setter
  def patch(self, patch):
    self._v[2] = patch
    self._dirty = True

def GetChecksum(contents):
  h = hashlib.sha256()
  h.update(contents)
  return base64.b16encode(h.digest())

def GetFileChecksum(file):
  """Return a SHA256 hash from the contents of the specified filename"""
  with open(file, "r") as fd:
    return GetChecksum(fd.read())

def CheckCall(args, cwd=None, quiet=False):
  with open(os.devnull, "w") as devnull:
    p = Popen(args, cwd=cwd,
              stdout = devnull if quiet == True else None,
              stderr = devnull if quiet == True else None)
    r = p.wait()
    if r is not 0: raise CalledProcessError(r, args)

def CheckOutput(args, cwd=None):
  e = os.environ
  e['LANG'] = 'C'
  p = Popen(args, cwd=cwd, stdout=PIPE, env=e)
  r = p.wait()
  if r is not 0: raise CalledProcessError(r, args)
  return p.stdout.read()

class ScopedTmpdir:
  def __init__(self, *args, **kwargs):
    self._args = args
    self._kwargs = kwargs

  def __enter__(self):
    self._tmpdir = tempfile.mkdtemp(*self._args, **self._kwargs)
    return self._tmpdir

  def __exit__(self, type, value, traceback):
    shutil.rmtree(self._tmpdir)

def LoadJsonFromPath(path, throw_on_failure = True):
  try:
    with open(path, "r") as fd:
      return json.load(fd)
  except:
    if throw_on_failure:
      raise
  return {}
