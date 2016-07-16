#!/usr/bin/env python

import sys

def tolong(p):
  return long(p, 0)

def report(testname, p, t, s):
  print 'Test: ' + testname
  for ptr, stats in p.iteritems():
    statsstr = ''.join(stats)
    if len(statsstr) % 2 != 0 or statsstr.count('a') != (len(statsstr)/2):
      print '  %s -> %s (%s)' % (ptr, ','.join(stats), ','.join(t[ptr]))

  for ptr, stats in t.iteritems():
    for i in range(0, len(stats) - 1, 2):
      if stats[i] != stats[i + 1]:
        print '  acrossMove: %s -> %s -> %s' % (ptr, ','.join(stats), ','.join(s[ptr]))

def reportptrrem(testname, pi, si, ti, spi, prefix):
  def printme(x):
    if printme.printedprefix == 0:
      print prefix
      printedprefix = 1  
    print x
  printme.printedprefix = 0

  statsstr = ''.join(si)
  if len(statsstr) % 2 != 0 or statsstr.count('a') != (len(statsstr)/2):
    printme('    %s -> %s (%s)' % (pi, ','.join(si), ','.join(ti)))

  for i in range(0, len(ti) - 1, 2):
    if ti[i] != ti[i + 1]:
      printme('    acrossMove: %s -> %s -> %s' % (pi, ','.join(ti), ','.join(spi)))

def removeptrsinrange(testname, ptr, sz, p, t, s):
  prefix = '  osUnmap - removing ptrs in range [%x, %x)' % (ptr, ptr + sz)
  torem = []
  for pi, si in p.iteritems():
    ptrint = tolong(pi)
    if ptrint >= ptr and ptrint < (ptr + sz):
      reportptrrem(testname, pi, si, t[pi], s[pi], prefix)
      torem.append(pi)

  for x in torem:
    del p[x]
    del t[x]
    del s[x]

if __name__ == '__main__':
  testname = ''
  p = { }
  t = { }
  s = { }
  with open(sys.argv[1]) as f:
    for line in f:
      line = line[:-1]
      if line.startswith('Test setup:'):
        testname = line.split(':')[1].strip().replace('\'', '')
        p = { }
        t = { }
        s = { }

      if line.startswith('osunmap'):
        toks = line.split(',')
        ptr = tolong(toks[1])
        sz = tolong(toks[2])
        removeptrsinrange(testname, ptr, sz, p, t, s)

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

      if line.startswith('Test cleanup:'):
        report(testname, p, t, s)
