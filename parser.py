#!/usr/bin/python

import sys
from sets import Set

f = open(sys.argv[1])

class canframe:

    def __init__(self, time, addr, payload):
        self.timeStr=time
        self.addrStr=addr
        self.payloadStr=payload
        self.addrBytes =  bytearray.fromhex(self.addrStr)
        self.payloadBytes = bytearray.fromhex(self.payloadStr)

    def __str__(self):
        return "Time: " + self.timeStr + "\nAddr: " + self.addrStr +"\nData: " + self.payloadStr
        

frames=[]
addrSet = Set()
fDict = {}

for line in f:
    s = line.split()
    t = s[0].strip('()')
    a = s[2].split('#')[0]
    p = s[2].split('#')[1]

    if a not in addrSet:
        fDict[a] = []

    addrSet.add(a)
    fDict[a].append( canframe(t, a, p) )
    frames.append( canframe(t, a, p) )


print frames[0]
print ""
print "Hex Addr: ", int("0x" + frames[0].addrStr, 16)
print "Hex Data: ", int("0x" + frames[0].payloadStr, 16)
print ""
print "Hex Addr: ", len(bytearray.fromhex(frames[0].addrStr))
print "Hex Data: ", len(bytearray.fromhex(frames[0].payloadStr))

for key in fDict:
    print fDict[key][0].addrStr, ": ", len(fDict[key])

#for item in fDict['20000004']:
#    print item.timeStr, ": ", item.payloadStr
