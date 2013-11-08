#!/usr/bin/python
# vim:expandtab:shiftwidth=2:tabstop=2:

# Copyright (C) 2013 Canonical Ltd.
# Copyright (C) 2013 The Chromium Authors

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

import inspect
from optparse import OptionParser
import textwrap

class CommandDispatcher(object):
  def __init__(self):
    self.handlers = {}

  def register_handler(self, command, handler):
    if command in self.handlers or command == "help":
      raise Exception("Handler for command '%s' registered more than once" %
                      command)

    if not inspect.isfunction(handler):
      raise Exception("Handler for command '%s' should be a function" %
                      command)

    if len(inspect.getargspec(handler).args) != 2:
      raise Exception("Handler for command '%s' accepts the wrong number of "
                      "arguments" % command)

    self.handlers[command] = handler

  def execute(self, args=[], parser=OptionParser()):
    if len(args) == 0:
      args.append("help")

    parser.format_description = lambda _: parser.description or ""
    parser.format_epilog = lambda _: parser.epilog or ""

    if args[0] in ("-h", "--help") and len(args) > 1:
      # Convert "--help command" in to "command --help"
      args = [args[1], args[0]] + args[2:]

    if args[0] == "help" and len(args) > 1:
      # Convert "help command" in to "command --help"
      args = [args[1], "--help"] + args[2:]

    try:
      handler = self.handlers[args[0]]
    except KeyError:
      handler = None

    if not handler:
      found_help = False
      for arg in args[1:]:
        if arg in ("-h", "--help"):
          found_help = True
          break
      if not found_help:
        args.insert(1, "--help")
      parser.set_usage("usage: %prog <command> [options]")
      if parser.description:
        parser.description += "\n\n"
      else:
        parser.description = ""
      commands = sorted((command, self._create_command_summary(handler))
                        for (command, handler) in self.handlers.iteritems())
      commands = [(command, doc) for (command, doc) in commands if doc]
      if len(commands) > 0:
        width = max(len(command) for (command, doc) in commands)
        parser.description += "Commands are:\n"
        parser.description += ("".join("  %-*s %s\n" % (width, command, doc)
                               for (command, doc) in commands))

      parser.parse_args(args[1:])
      assert False

    for option in getattr(handler, "_subcommand_options", []):
      parser.add_option(*option[0], **option[1])

    parser.set_usage("usage: %%prog %s [options]" % handler._subcommand_name)
    if handler._subcommand_usage_more is not None:
      parser.set_usage("%s%s" % (parser.get_usage().strip(),
                                 handler._subcommand_usage_more))

    parser.description = ""
    if handler.__doc__:
      lines = handler.__doc__.rstrip().splitlines()
      if lines[:1]:
        rest = textwrap.dedent("\n".join(lines[1:]))
        parser.description = "\n".join((lines[0], rest))
      else:
        parser.description = lines[0]
      if parser.description:
        parser.description += "\n"

    parser.epilog = getattr(handler, "_subcommand_epilog", None)
    if parser.epilog:
      parser.epilog("\n%s\n" % parser.epilog.strip())

    (options, args) = parser.parse_args(args[1:])

    return handler(options, args)

  def _create_command_summary(self, handler):
    doc = handler.__doc__ or ""
    line = doc.split("\n", 1)[0].rstrip(".")
    if not line:
      return line
    return (line[0].lower() + line[1:]).strip()

Dispatcher = CommandDispatcher()

class Command(object):
  def __init__(self, command, usage_more=None, epilog=None):
    self.command = command
    self.epilog = epilog
    self.usage_more = usage_more

  def __call__(self, func):
    Dispatcher.register_handler(self.command, func)
    func._subcommand_name = self.command
    func._subcommand_epilog = self.epilog
    func._subcommand_usage_more = self.usage_more

    return func

class CommandOption(object):
  def __init__(self, *args, **kwargs):
    self.option = (args, kwargs)

  def __call__(self, func):
    options = getattr(func, "_subcommand_options", [])
    options.append(self.option)
    func._subcommand_options = options

    return func
