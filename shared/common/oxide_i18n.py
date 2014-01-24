#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
Parse webkit_strings.grd and output a header file containing
a function that maps message IDs to gettext-localized strings.
"""

import argparse
import codecs
import os.path
import sys

def parse_webkit_strings(webkit_strings, chromiumsrcdir):
  msgs = []
  if chromiumsrcdir is not None:
    sys.path.insert(0, os.path.join(chromiumsrcdir, 'tools', 'grit'))
  from grit import grd_reader, util
  from grit.node import empty, message
  res = grd_reader.Parse(webkit_strings)
  root = res.GetRoot()
  try:
    substituter = root.GetSubstituter()
  except AttributeError:
    substituter = None
  lang = 'en'
  for item in root.ActiveDescendants():
    if isinstance(item, empty.MessagesNode):
      for subitem in item.ActiveDescendants():
        if isinstance(subitem, message.MessageNode):
          key = subitem.GetTextualIds()[0]
          value = subitem.ws_at_start + subitem.Translate(lang) + subitem.ws_at_end
          # Escape quotation marks
          value = value.replace('"', '\\"')
          # Replace linebreaks with a \n escape
          value = util.LINEBREAKS.sub(r'\\n', value)
          if substituter is not None:
            value = substituter.Substitute(value)
          comment = subitem.attrs['meaning']
          msgs.append({'key': key, 'value': value, 'comment': comment})
  return msgs

def output_strings_header(filename, msgs):
  cases = ''
  for msg in msgs:
    cases += u'  case {0}:\n'.format(msg['key'])
    if msg['comment']:
      cases += u'    // TRANSLATORS: {0}\n'.format(msg['comment'])
    cases += u'    return C::gettext("{0}");\n'.format(msg['value'])
  cases += u'  default:\n    return "";'
  return u"""// This file is automatically generated from {filename}.
// Do not edit.

#ifndef {guard}
#define {guard}

namespace C {{
#include <libintl.h>
}}

#include "webkit/grit/webkit_strings.h"

static const char* localized_message_from_id(int message_id) {{
  switch(message_id) {{
{cases}
  }}
}}

#endif // {guard}
""".format(filename=filename, cases=cases, guard='_OXIDE_I18N_MESSAGES_H_')

def write_output_to_file(output, filepath):
  with codecs.open(filepath, encoding='utf-8', mode='w') as f:
    f.write(output)

if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument('-c', dest='chromiumsrcdir')
  parser.add_argument('-i', dest='input', required=True)
  parser.add_argument('-o', dest='output', required=True)
  args = parser.parse_args()
  msgs = parse_webkit_strings(args.input, args.chromiumsrcdir)
  filename = os.path.basename(args.input)
  output = output_strings_header(filename, msgs)
  write_output_to_file(output, args.output)
