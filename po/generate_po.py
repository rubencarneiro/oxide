#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
Parse webkit_strings.grd and output PO files for all available languages.
"""

import argparse
from datetime import datetime
import os.path
import sys

import polib

def parse_webkit_strings(webkit_strings):
  from grit import grd_reader
  return grd_reader.Parse(webkit_strings)

def extract_translations(res, potfile, outdir):
  from grit.node import message
  res.SetOutputLanguage('en')
  res.RunGatherers()
  for output in res.GetOutputFiles():
    if output.GetType() != 'data_package':
      continue
    lang = output.GetLanguage()
    if lang == '' or lang.startswith('fake'):
      continue
    res.SetOutputLanguage(output.GetLanguage())
    po = polib.pofile(potfile)
    po.metadata['Language'] = lang.split('-')[0]
    po.metadata['PO-Revision-Date'] = datetime.utcnow().strftime('%Y-%m-%d %H:%M+0000')
    po.metadata['Last-Translator'] = ''
    po.metadata['Language-Team'] = ''
    for node in res.ActiveDescendants():
      if isinstance(node, message.MessageNode):
        key = node.GetTextualIds()[0]
        msg = node.Translate('en')
        id, translated = node.GetDataPackPair(lang, 1)
        for entry in po:
          if entry.msgctxt == key:
            entry.msgstr = translated.decode('utf-8')
            break
    po.save(os.path.join(outdir, '{}.po'.format(lang.replace('-', '_'))))

if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument('-c', dest='chromiumsrcdir')
  parser.add_argument('-i', dest='input', required=True)
  parser.add_argument('-p', dest='potfile', required=True)
  parser.add_argument('-o', dest='outdir', required=True)
  args = parser.parse_args()
  if args.chromiumsrcdir is not None:
    sys.path.insert(0, os.path.join(args.chromiumsrcdir, 'tools', 'grit'))
  res = parse_webkit_strings(args.input)
  extract_translations(res, args.potfile, args.outdir)

