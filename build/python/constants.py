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

import os.path

# OXIDESRC_DIR is ../.. from this file
OXIDESRC_DIR = os.path.abspath(os.path.join(__file__, os.pardir, os.pardir, os.pardir))

# TOPSRC_DIR is OXIDESRC_DIR/..
TOPSRC_DIR = os.path.abspath(os.path.dirname(OXIDESRC_DIR))

OXIDEDEPS_FILE = os.path.join(OXIDESRC_DIR, "DEPS.oxide")

# TOP_DIR is TOPSRC_DIR/.., and contains .gclient and src/ in a full checkout
TOP_DIR = os.path.abspath(os.path.dirname(TOPSRC_DIR))

TOPSRC_DIRNAME = "src"
