#!/usr/bin/python

import sys
from sets import Set

# Could make an object for a collection of canframes:
# - One address, multiple frames - could be called "canDevice"?
# - Could merge entropy calculation into it

class canframe:

    def __init__(self, time, addr, payload):
        
        # String data
        self.timeStr=time
        self.addrStr=addr
        self.payloadStr=payload
        self.csv = time + ',' + addr + ',' + payload + '\n'
        
        # Parse data into bytes
        self.addrBytes =  bytearray.fromhex(self.addrStr)
        self.payloadBytes = bytearray.fromhex(self.payloadStr)
        self.payloadByteStr = []

        for b in self.payloadBytes:
            self.payloadByteStr.append(hex(b))

        self.payloadByteStr = ','.join(self.payloadByteStr)

    def __str__(self):
        return "Time: " + self.timeStr + "\nAddr: " + self.addrStr +"\nData: " + self.payloadStr

    def makeCsv(self):
        #return ','.join([self.timeStr, self.addrStr, self.payloadStr]) + '\n'
        return ','.join([self.timeStr, self.addrStr, self.payloadStr, self.payloadByteStr]) + '\n'
        
def writeCSV(frameArray, filename):
    fo = open(filename, 'w+')
    for item in frameArray:
        fo.write( item.makeCsv() )

def findEntropy(dictionary, interestingOnly):
    for addr in dictionary:
        e = Set()
        for frame in dictionary[addr]:
            e.add( frame.payloadStr )
        entropy = float(len(e)) / float(len(dictionary[addr]))
        if interestingOnly:
            if len(e) > 1 and len(dictionary[addr]) > 1:
                print '0x{0} Unique: {2:4d}, Total: {3:4d} Entropy: {1:.2%}'.format(addr, entropy, len(e), len(dictionary[addr]))
        else:
                print '0x{0} Unique: {2:4d}, Total: {3:4d} Entropy: {1:.2%}'.format(addr, entropy, len(e), len(dictionary[addr]))

def uniqueAddresses(dictionary):
    l = []
    for key in dictionary:
        l.append(key)
    return l

def parseLog(f):

    addrSet = Set()

    for line in f:
        s = line.split()
        t = s[0].strip('()')
        a = s[2].split('#')[0]
        p = s[2].split('#')[1]

        if a not in addrSet:
            frameDict[a] = []

        addrSet.add(a)
        frameDict[a].append( canframe(t, a, p) )
        frames.append( canframe(t, a, p) )


### Parse argument for log file and directory
path = sys.argv[1].rpartition('/')
workingDir = path[0] + '/'
filename = path[2]

### Variables
frames=[]
frameDict = {}
interestingOnly = True  # makes entropy command only print out addresses with data entropy > 1

parseLog( open( workingDir + filename ) )

# This function should return some sort of list or dictionary, which we then can print.
findEntropy( frameDict, interestingOnly )

# Parse all data into a CSV file.  Keeps same name as log file.
csvFile = workingDir + '/' + filename[:len(filename)-3] + 'csv'
writeCSV(frames, csvFile)
print "Parsed log into CSV: ", csvFile
