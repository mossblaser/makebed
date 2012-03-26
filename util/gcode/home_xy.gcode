(Power on)
M-1

(mm)
G21

(Set the Z axis as we're not homing that)
G92 X0 Y0 Z10

(Lift the head out of its hole)
G1 Z15 F100

(Home x[1] and y[2] at the same time[1+2 = 3])
M-2 A3

(Move to the calibration ring)
G1 X-56 Y-44 Z15 F3300
G1 Z10 F100

(Power off)
M0
