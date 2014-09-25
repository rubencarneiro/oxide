#!/usr/bin/python
# vim:expandtab:shiftwidth=2:tabstop=2:

# Copyright (C) 2014 Canonical Ltd.

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

from collections import deque
import logging
import os
from select import select

def _fd_from_fileobj(fileobj):
  fd = -1
  if isinstance(fileobj, int):
    return fileobj
  else:
    try:
      return int(fileobj.fileno())
    except (AttributeError, TypeError):
      raise ValueError("File object does not implement fileno() method")

class Handler:
  def __init__(self, handler, args):
    self._handler = handler
    self._args = args
    self._cancelled = False

  def cancel(self):
    self._cancelled = True

  def _run(self):
    assert not self._cancelled
    try:
      self._handler(*self._args)
    except Exception as e:
      logging.error("Task threw exception: %s" % str(e))

class EventLoop:
  def __init__(self):
    self._ready_tasks = deque()
    self._pending_tasks = deque()

    self._read_fds = set()
    self._read_fd_to_handler_map = {}

    self._is_running = False
    self._quit_now = False
    self._quit_when_idle = False
    self._is_closed = False

    (r, w) = os.pipe()
    self._wakeup_read_fo = os.fdopen(r, "r")
    self._wakeup_write_fo = os.fdopen(w, "w")

    self.add_reader(self._wakeup_read_fo, self._read_from_self)

  def run(self):
    self._run_internal(False)

  def run_until_idle(self):
    self._run_internal(True)

  def quit(self):
    self._quit_now = True
    self._wake()

  def quit_when_idle(self):
    self._quit_when_idle = True
    self._wake()

  def close(self):
    assert not self._is_running

    fds = self._read_fds.copy()
    while len(fds) > 0:
      self.remove_reader(fds.pop())

    assert len(self._read_fds) == 0
      
    self._wakeup_read_fo.close()

    self._ready_tasks.clear()

  def call_soon(self, callback, *args):
    handler = Handler(callback, args)
    self._pending_tasks.append(handler)
    self._wake()
    return handler

  def add_reader(self, reader, callback, *args):
    fd = _fd_from_fileobj(reader)
    if fd < 0:
      raise ValueError("Invalid file descriptor")
    self._read_fd_to_handler_map[fd] = Handler(callback, args)
    self._read_fds.add(fd)
    self._wake()

  def remove_reader(self, reader):
    fd = _fd_from_fileobj(reader)
    if fd < 0:
      raise ValueError("Invalid file descriptor")
    if fd not in self._read_fds:
      raise ValueError("File descriptor is not in the list of read descriptors")
    self._read_fds.remove(fd)
    del self._read_fd_to_handler_map[fd]
    self._wake()

  def _read_from_self(self):
    try:
      self._wakeup_read_fo.read()
    except:
      pass

  def _wake(self):
    self._wakeup_write_fo.write("A")

  def _run_internal(self, until_idle):
    if self._is_running:
      raise Exception("Loop is already running")
    if self._is_closed:
      raise Exception("Cannot run a closed loop")

    try:
      self._is_running = True
      self._quit_now = False
      self._quit_when_idle = until_idle

      while True:
        if self._run_once(False) and not self._quit_now:
          continue
        if self._quit_now or self._quit_when_idle:
          break
        self._run_once(True)
        if self._quit_now:
          break

    finally:
      self._is_running = False

  def _run_once(self, can_block):
    if len(self._ready_tasks) == 0:
      timeout = None
      if not can_block:
        timeout = 0
      (r, w, x) = select(self._read_fds, [], [], timeout)
      for fd in r:
        self._ready_tasks.append(self._read_fd_to_handler_map[fd])

      while len(self._pending_tasks) > 0:
        self._ready_tasks.append(self._pending_tasks.popleft())

    while len(self._ready_tasks) > 0:
      handler = self._ready_tasks.popleft()
      if handler._cancelled:
        continue
      handler._run()
      return True

    return False
