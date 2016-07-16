#!/usr/bin/env python

import sys

if __name__ == '__main__':
  p = { }
  with open(sys.argv[1]) as f:
    for line in f:
      if len(line) >= 2 and line[1] == ',':
        toks = line.split(',')
        if toks[2] in p:
          p[toks[2]].append(toks[0])
        else:
          p[toks[2]] = [ toks[0] ]

  for ptr, stats in p.iteritems():
    statsstr = ''.join(stats)
    if len(statsstr) % 2 != 0 or statsstr.count('a') != (len(statsstr)/2):
      print '%s -> %s' % (ptr, ','.join(stats))
