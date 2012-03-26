#!/usr/bin/python

from math import *

print """
G21
G92 X0 Y0 Z0
"""

radius = 40.0
segments = 40000
feedrate = 330

for seg in range(segments):
	angle = ((2*pi) * seg) / segments
	print "G1 X%f Y%f F%f"%(cos(angle)*radius, sin(angle)*radius, feedrate)

print "G1 X0 Y0 F%f"%feedrate
