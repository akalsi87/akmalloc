#!/usr/bin/env python

import sys

if __name__ == '__main__':
  p = { }
  t = { }
  s = { }
  with open(sys.argv[1]) as f:
    for line in f:
      line = line[:-1]
      if len(line) >= 2 and line[1] == ',':
        toks = line.split(',')
        if toks[2] in p:
          p[toks[2]].append(toks[0])
          t[toks[2]].append(toks[1])
          s[toks[2]].append(toks[3])
        else:
          p[toks[2]] = [ toks[0] ]
          t[toks[2]] = [ toks[1] ]
          s[toks[2]] = [ toks[3] ]

  for ptr, stats in p.iteritems():
    statsstr = ''.join(stats)
    if len(statsstr) % 2 != 0 or statsstr.count('a') != (len(statsstr)/2):
      print '%s -> %s' % (ptr, ','.join(stats))

  for ptr, stats in t.iteritems():
    for i in range(0, len(stats) - 1, 2):
      # print '%s -> %s' % (ptr, ','.join(stats))
      if stats[i] != stats[i + 1]:
        print 'WARNING: %s -> %s -> %s' % (ptr, ','.join(stats), ','.join(s[ptr]))
