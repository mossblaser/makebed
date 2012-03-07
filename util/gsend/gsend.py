#!/usr/bin/python

import socket
import time
import ctypes

zero_window_poll_rate = 0.05

# Unpack data from bytes into a given ctype
def from_bytes(container, bytes):
	fit = min(len(bytes), ctypes.sizeof(container))
	ctypes.memmove(ctypes.addressof(container), bytes, fit)
	return container


class GSender(object):
	
	def __init__(self, ip, port):
		# Record the target
		self.target = (ip, port)
		
		# Open a UDP socket
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.sock.settimeout(zero_window_poll_rate)
		
		# Internal state
		self.window_size = 0
		self.seq_num = 10
	
	def send(self, data):
		while data:
			packet = data[:max(self.window_size-4,0)]
			data   = data[max(self.window_size-4,0):]
			
			# Prepend the sequence number
			packet = buffer(ctypes.c_uint32(self.seq_num)) + packet
			
			# Transmit until ack'd
			while True:
				self.sock.sendto(packet, self.target)
				
				try:
					response, _ = self.sock.recvfrom(8)
					seq_num     = from_bytes(ctypes.c_uint32(), response[0:4])
					window_size = from_bytes(ctypes.c_uint32(), response[4:8])
				except socket.timeout:
					# Retransmit
					continue
				
				if seq_num.value == self.seq_num:
					self.window_size = window_size.value
					seq_num.value += 1
					self.seq_num = seq_num.value
					break
				else:
					# Retransmit
					pass
			
			if self.window_size == 0:
				time.sleep(zero_window_poll_rate)



gs = GSender("192.168.3.100", 1818)

import sys
gs.send(sys.stdin.read())
