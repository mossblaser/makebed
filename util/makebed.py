#!/usr/bin/python
# Front-end for controling the printer

import os, sys, time, optparse
import gsend, debug


def makebed_send(ip, *args):
	# Get options
	parser = optparse.OptionParser()
	parser.add_option("-f", "--file",
	                  action="store", type="string", dest="filename",
	                  help="File to send to the printer (default stdin)")
	parser.add_option("-p", "--poll_rate",
	                  action="store", type="float", dest="poll_rate",
	                  default = gsend.DEFAULT_POLL_RATE,
	                  help="Rate at which to poll the printer when busy (sec).")
	parser.add_option("-v", "--verbose",
	                  action="store_true", dest="verbose", default=False,
	                  help="Show detailed information about the connection")
	options, args = parser.parse_args(list(args))
	
	options.filename = options.filename or (args[0] if len(args) >= 1 else None)
	f = open(options.filename, "r") if options.filename else sys.stdin
	
	# Start the transfer
	gsend.GSender(ip).send(f, poll_rate = options.poll_rate, verbose = options.verbose)


def makebed_get(ip, *args):
	# Get options
	parser = optparse.OptionParser(usage="[-p] [temp|gcode|buf|pow|pos|msg]...")
	parser.add_option("-p", "--poll_rate",
	                  action="store", type="float", dest="poll_rate",
	                  default = None,
	                  help="Repeatedly request data.")
	options, args = parser.parse_args(list(args))
	
	# Connect
	d = debug.Debugger(ip)
	
	commands = {
		"temp": (
			"extruder_c	extruder_target_c	platform_c	platform_target_c",
			d.get_temperatures
		),
		"gcode": (
			"gcode_cmds_parsed",
			d.get_instructions_parsed
		),
		"buf": (
			"gcode_buf	gcode_buf_size	cmd_buf	cmd_buf_size	cmd_buf_underruns",
			d.get_buffer_stats
		),
		"pow": (
			"power_on",
			d.get_power
		),
		"pos": (
			"X	Y	Z",
			d.get_position
		),
		"msg": (
			"message",
			d.get_message
		),
	}
	
	selected = []
	for arg in args:
		if arg in commands:
			selected.append(commands[arg])
		else:
			raise Exception("Unknown field: '%s'"%arg)
	
	sys.stdout.write("#" + "\t".join(map((lambda c:c[0]), selected)) + "\n")
	sys.stdout.flush()
	
	while True:
		sys.stdout.write("\t".join(map((lambda (_,f): "\t".join(map(str, f()))), selected)) + "\n")
		sys.stdout.flush()
		
		if options.poll_rate is None:
			break
		else:
			time.sleep(options.poll_rate)


def makebed_reset(ip, *args):
	print "TODO"


################################################################################


# Remove the script name
sys.argv.pop(0)

# Get the IP of the printer
try:
	ip = open(os.path.expanduser("~/.makebedrc"), "r").read().strip()
except IOError, e:
	print "Could not read printer IP from ~/.makebedrc", e
	sys.exit(1)


commands = {
	"send"  : makebed_send,
	"get"   : makebed_get,
	"reset" : makebed_reset,
}

# Call the specified command
commands[sys.argv.pop(0)](ip, *sys.argv)
