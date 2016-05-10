#!/usr/bin/python
# vim:expandtab:shiftwidth=2:tabstop=2:

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

from ConfigParser import ConfigParser, NoOptionError
import os.path

from constants import TOP_DIR

CONFIGFILE_PATH = os.path.join(TOP_DIR, ".checkout.cfg")

class Config(ConfigParser):
  def __init__(self):
    ConfigParser.__init__(self, allow_no_value=True)

    with open(CONFIGFILE_PATH, "r") as f:
      self.readfp(f)

  @property
  def cachedir(self):
    try:
      return self.get("DEFAULT", "cachedir")
    except NoOptionError:
      return None

  @cachedir.setter
  def cachedir(self, value):
    self.set("DEFAULT", "cachedir", value)

  @property
  def url(self):
    try:
      return self.get("DEFAULT", "url")
    except NoOptionError:
      return None

  def save(self):
    with open(CONFIGFILE_PATH, "w") as f:
      self.write(f)

def ConfigExists():
  return os.path.isfile(CONFIGFILE_PATH)
