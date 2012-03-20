#!/usr/bin/python
# Library for sending data via the UDP hack

import socket
import time

class Debugger(object):
	
	def __init__(self, ip, port = 2777):
		# Record the target
		self.target = (ip, port)
		
		# Open a TCP connection
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.sock.connect(self.target)
	
	
	def read_line(self):
		char = None
		buf = ""
		
		while (char or " ") not in "\n\r":
			buf += char or ""
			char = self.sock.recv(1)
			
			if char == "":
				raise Exception("Disconnected!")
				break
		
		return buf
	
	
	def send_command(self, cmd):
		self.sock.send(cmd + "\n")
		return self.read_line().strip()
	
	
	def get_temperatures(self):
		response = self.send_command("tmp").split(" ")
		return (
			float(response[0]) / 100.0,
			float(response[1]) / 100.0,
			int(response[2]),
			float(response[3]) / 100.0,
			float(response[4]) / 100.0,
			int(response[5]),
		)
	
	
	def get_instructions_parsed(self):
		return [int(self.send_command("gcd"))]
	
	
	def get_buffer_stats(self):
		return map(int, self.send_command("buf").split(" "))
	
	
	def get_power(self):
		return [bool(self.send_command("pow"))]
	
	
	def get_position(self):
		return map((lambda t: int(t)/100.0), self.send_command("pos").split(" "))
	
	
	def get_message(self):
		return [self.send_command("dbg")]


