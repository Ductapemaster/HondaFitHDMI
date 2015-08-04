#!/usr/bin/python

import sys

f = open(sys.argv[0])

class canframe:

    def __init__(self, time, addr, payload):
        self.time=time
        self.addr=addr
        self.payload=payload

frames=dict()

for line in f:
    print line.split()

