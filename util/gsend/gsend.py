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
		#self.sock.bind(("127.0.0.1", port + 1))
		
		# Internal state
		self.window_size = 0
	
	def send(self, data):
		while data:
			packet, data = data[:self.window_size], data[self.window_size:]
			
			self.sock.sendto(packet, self.target)
			response, _ = self.sock.recvfrom(4)
			self.window_size = from_bytes(ctypes.c_uint32(), response).value
			
			if self.window_size == 0:
				time.sleep(zero_window_poll_rate)



gs = GSender("192.168.3.100", 1818)

import sys
gs.send(sys.stdin.read())
