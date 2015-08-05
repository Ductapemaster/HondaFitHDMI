#!/usr/bin/python

import sys
from sets import Set

# Could make an object for a collection of canframes:
# - One arbitration ID, multiple frames - could be called "canDevice"?
# - Could merge entropy calculation into it

class canframe:

    def __init__(self, time, arbId, payload):
        
        # String data
        self.timeStr=time
        self.arbIdStr=arbId
        self.payloadStr=payload
        self.csv = time + ',' + arbId + ',' + payload + '\n'
        
        # Parse data into bytes
        self.arbIdBytes =  bytearray.fromhex(self.arbIdStr)
        self.payloadBytes = bytearray.fromhex(self.payloadStr)
        self.payloadByteStr = []

        for b in self.payloadBytes:
            self.payloadByteStr.append(hex(b))

        self.payloadByteStr = ','.join(self.payloadByteStr)

    def __str__(self):
        return "Time: " + self.timeStr + "\narbId: " + self.arbIdStr +"\nData: " + self.payloadStr

    def makeCsv(self):
        #return ','.join([self.timeStr, self.arbIdStr, self.payloadStr]) + '\n'
        return ','.join([self.timeStr, self.arbIdStr, self.payloadStr, self.payloadByteStr]) + '\n'
        
def writeCSV(frameArray, filename):
    fo = open(filename, 'w+')
    for item in frameArray:
        fo.write( item.makeCsv() )

def findPayloadEntropy(canFrameArray):

    e = Set()

    for frame in canFrameArray:
        e.add( frame.payloadStr )

    entropy = float( len( e ) ) / float( len( canFrameArray ) )
    
    return e, entropy


def uniqueArbIDs(dictionary):

    l = []

    for key in dictionary:
        l.append(key)

    return l

def parseLog(f):

    arbIdSet = Set()

    for line in f:
        s = line.split()
        t = s[0].strip('()')
        a = s[2].split('#')[0]
        p = s[2].split('#')[1]

        if a not in arbIdSet:
            frameDict[a] = []

        arbIdSet.add(a)
        frameDict[a].append( canframe(t, a, p) )
        frames.append( canframe(t, a, p) )


### Parse argument for log file and directory
path = sys.argv[1].rpartition('/')
workingDir = path[0] + '/'
inFile = path[2]

### Variables
frames=[]               # Generic parsed form of data
frameDict = {}          # Dictionary of CAN Frames accessed by arbitration ID:
interestingOnly = True  # makes entropy command only print out Arbitration IDs with data entropy > 1

parseLog( open( workingDir + inFile ) )

# Look through dictionary and find interesting values
for arbId in frameDict:
    payloadSet, entropy = findPayloadEntropy( frameDict[arbId] )
    if len(payloadSet) > 1 and len( frameDict[arbId] ) > 1:
        print '0x{0} Unique: {2:4d}, Total: {3:4d} Entropy: {1:.2%}'.format(arbId, entropy, len(payloadSet), len( frameDict[arbId] ))

# Parse all data into a CSV file.  Keeps same name as log file.
csvFile = workingDir + '/' + inFile[:len(inFile)-3] + 'csv'
writeCSV(frames, csvFile)
print "Parsed log into CSV: ", csvFile
