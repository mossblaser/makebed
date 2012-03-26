(mm)
G21

(Start in parking position)
G92 X-60 Y45 Z10

(Raise up to avoid the loop)
G1 Z12 F100

(Move to squirt position)
G1 X-55 Y10 F1000
G1 Z7 F100

(Extrude a bit and stop)
M101
G4 P5000
M103
G4 P6000

(Wipe)
G1 Y-10 F2000

(Return to home)
G1 Z12 F100
G1 X-60 Y45 F3300
G1 Z10 F100

