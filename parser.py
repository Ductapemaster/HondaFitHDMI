#!/usr/bin/python

import sys

f = open(sys.argv[1])

class canframe:

    def __init__(self, time, addr, payload):
        self.time=time
        self.addr=addr
        self.payload=payload

frames=[]

for line in f:
    s = line.split()
    t = s[0].strip('()')
    a = s[2].split('#')[0]
    p = s[2].split('#')[1]
    frames.append( canframe(t, a, p) )

print "Time: ", frames[0].time
print "Addr: ", frames[0].addr
print "Data: ", frames[0].payload

