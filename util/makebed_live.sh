#!/bin/bash

# How long to look back through the data
DURATION="$(( 60 * 10 ))"

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
reset
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
set xrange [-60:60]
set yrange [-60:60]
set zrange [0:120]
set autoscale cb
set palette defined (0 "white", 0.9 "yellow", 1 "red")
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
		
		# After the "key", insert zeros for the first DURATION samples
		if echo "$line" | grep -qREx "#.*"; then
			zeros="$(echo "$line" | tr -d "#" | sed -re "s/[a-zA-Z_]+/0/g")"
			
			echo "# *** Begin initial zero padding ***" >> "$LOGFILE"
			for i in `seq $DURATION`; do
				echo $zeros >> "$LOGFILE"
			done
			echo "# *** End initial zero padding ***" >> "$LOGFILE"
			
			echo "# Mark discontinuity" >> "$LOGFILE"
			echo >> "$LOGFILE"
		fi
		
		echo "$line" > /dev/stderr
		echo "$CMDS"
		
	done
) | gnuplot

rm "$LOGFILE"
