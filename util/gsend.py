#!/usr/bin/python
# Library for sending data via the UDP hack

import socket
import time
import ctypes
import sys

DEFAULT_POLL_RATE = 0.1

class Rejected(Exception):
	pass


class GSender(object):
	
	def __init__(self, ip, port = 1818):
		# Record the target
		self.target = (ip, port)
		
		# Open a UDP socket
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		
		# Connection state
		self.window_size = 0
		self.seq_num     = 0
		
		# Stat counters
		self.err_response_timeout = 0
		self.err_wrong_seq_num = 0
		self.data_sent = 0
	
	
	@property
	def data_window_size(self):
		# Size of the window less the size of the headder
		return max(self.window_size - 4, 0)
	
	
	# Unpack data from bytes into a given ctype
	def from_bytes(self, container, bytes):
		fit = min(len(bytes), ctypes.sizeof(container))
		ctypes.memmove(ctypes.addressof(container), bytes, fit)
		return container
	
	
	# Generate a 4-byte uint32 string
	def uint32_to_str(self, value):
		return buffer(ctypes.c_uint32(value))
	
	
	# Get a uint32 out of a 4-byte string
	def str_to_int(self, dat):
		return self.from_bytes(ctypes.c_uint32(), dat).value
	
	
	def send(self, f, poll_rate = DEFAULT_POLL_RATE, verbose = False):
		self.sock.settimeout(poll_rate)
		
		eof = False
		
		if verbose:
			sys.stderr.write("Contacting Printer")
			sys.stderr.flush()
		
		# While there is still data remaining
		while not eof:
			# Read in a packet worth of data and add a sequence number
			packet = self.uint32_to_str(self.seq_num) + f.read(self.window_size)
			eof = len(packet) == 4 and self.window_size > 0
			
			self.data_sent += len(packet) - 4
			
			if verbose and len(packet) - 4 > 4:
				sys.stderr.write("\nSent %3d byte packet, %0.1fkB total"%(len(packet) - 4,
				                                                          self.data_sent / 1024.0))
				sys.stderr.flush()
			
			# Transmit until ack'd
			while True:
				# Send the packet
				self.sock.sendto(packet, self.target)
				
				# Get the response packet
				try:
					response, _ = self.sock.recvfrom(8)
					seq_num     = self.str_to_int(response[0:4])
					window_size = self.str_to_int(response[4:8])
				except socket.timeout:
					# No response within timeout, retransmit
					self.err_response_timeout += 1
					if verbose:
						sys.stderr.write("\nNo response. Retrying... (%d)"%(self.err_response_timeout))
						sys.stderr.flush()
					continue
				
				if self.seq_num != 0 and seq_num == 0 and window_size == 0:
					# The device rejected the packet
					raise Rejected("Device rejected data!")
				elif seq_num == self.seq_num:
					# The sequence number of the response matched
					if verbose:
						if seq_num == 0 and window_size == 0:
							sys.stderr.write("\nWaiting for buffer to empty.")
							sys.stderr.flush()
						elif window_size == 0:
							sys.stderr.write(".")
							sys.stderr.flush()
					
					self.window_size = window_size
					self.seq_num = seq_num + 1
					
					# Break out of the retransmit loop and send the next packet
					break
				else:
					# The response was from some other packet, retransmit
					self.err_wrong_seq_num += 1
					if verbose:
						sys.stderr.write("\nRecieved unexpected message. Retrying... (%d)"%(self.err_wrong_seq_num))
						sys.stderr.flush()
					continue
			
			# If a zero windowsize was reported, wait a while and try again
			if self.window_size == 0:
				time.sleep(poll_rate)
		
		if verbose:
			sys.stderr.write("\nDone! Total: %0.1fkB.\n"%(self.data_sent / 1024.0))
			sys.stderr.flush()

