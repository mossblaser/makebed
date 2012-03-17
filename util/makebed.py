#!/usr/bin/python
# Front-end for controling the printer

import os, sys, gsend, optparse


def makebed_send(ip, *args):
	# Get options
	parser = optparse.OptionParser()
	parser.add_option("-f", "--file",
	                  action="store", type="string", dest="filename",
	                  help="File to send to the printer (default stdin)")
	parser.add_option("-v", "--verbose",
	                  action="store_true", dest="verbose", default=False,
	                  help="Show detailed information about the connection")
	options, args = parser.parse_args(list(args))
	
	options.filename = options.filename or (args[0] if len(args) >= 1 else None)
	f = open(options.filename, "r") if options.filename else sys.stdin
	
	# Start the transfer
	gsend.GSender(ip).send(f, verbose = options.verbose)


def makebed_get(ip, args):
	print "TODO"


def makebed_reset(ip, args):
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
