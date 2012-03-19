#!/bin/bash

# How long to look back through the data
DURATION="$(( 60 * 2 ))"

# Buffer sizes
GCODE_BUF_SIZE=$(( (2048 * 110) / 100 ))
CMD_BUF_SIZE=$(( (25 * 110) / 100 ))

LOGFILE="$(mktemp)"
echo "Using log file: $LOGFILE"

TRUNK_LOGFILE="< tail -n'$(( $DURATION + 1 ))' '$LOGFILE'"
LAST_LOGFILE="< tail -n1 '$LOGFILE'"

INITIAL_CMDS="$(cat <<EOF
set term wxt noraise
EOF
)"

CMDS="$(cat <<EOF
set multiplot layout 2,2

set xlabel "Time (Seconds)"
set ylabel "Temperature (C)"
set xrange [0:$DURATION]
set yrange [0:10]
set autoscale ymax
set cbrange [0:1]
set palette defined (0 "black", 1 "red")
unset colorbox

set title "Extruder Temperature"
plot "$TRUNK_LOGFILE" using 0:1:3 title "Current" with lines palette, \
     "$TRUNK_LOGFILE" using 2 title "Target" with lines

set title "Platform Temperature"
plot "$TRUNK_LOGFILE" using 0:4:6 title "Current" with lines palette, \
     "$TRUNK_LOGFILE" using 5 title "Target" with lines

set title "Platform Position"
set xlabel "X (mm)"
set ylabel "Y (mm)"
set zlabel "Z (mm)"
set xrange [-30:30]
set yrange [-30:30]
set zrange [0:50]
set autoscale cb
set palette defined (0 "white", 0.2 "yellow", 1 "red")
unset colorbox
splot "$TRUNK_LOGFILE" using 12:13:14:0 notitle with lines palette, \
      "$LAST_LOGFILE" using 12:13:14:0 notitle

set title "Buffer Usage"
set xlabel "Time (Seconds)"
set ylabel "Gcode Buffer (Bytes)"
set y2label "Command Buffer (Commands)"
set xrange [0:$DURATION]
set yrange [0:$GCODE_BUF_SIZE]
set y2range [0:$CMD_BUF_SIZE]
set ytics nomirror
set y2tics
plot "$TRUNK_LOGFILE" using 7 title "Gcode" with lines, \
     "$TRUNK_LOGFILE" using 9 title "Commands" with lines axes x1y2

EOF
)"


python makebed.py get -p 1 temp buf pos | (
	echo "$INITIAL_CMDS"
	
	while read line; do
		echo "$line" >> "$LOGFILE"
		echo "$line" > /dev/stderr
		echo "$CMDS"
		
	done
)| gnuplot

rm "$LOGFILE"
