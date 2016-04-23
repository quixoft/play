#!	/usr/bin/python
#	coding: utf8

"""
Talk to the CosmicPi Arduino DUE accross USB
This program has four main functions
1) Build event messages and send them to a server
2) Perform diagnostics and monitoring
3) Log events to the log file
4) Execute commnads on the Arduino 

Typing the '>' character turns on command input

julian.lewis lewis.julian@gmail.com 15/Feb/2016
"""

import sys
import socket
import select
import serial
import time
import traceback
import os
import termios
import fcntl
import re
from optparse import OptionParser

def main():
	ser = serial.Serial(port="/dev/ttyACM0", baudrate=9600, timeout=60)
	while (1):
		rc = ser.readline();
		if rc.find("EOF") != -1:
			break
	
	while (1):
		rc = ser.readline();
		if rc.find("EOF") != -1:
			break
		else:
			sys.stdout.write(rc)
			sys.stdout.flush()
		
if __name__ == '__main__':

	main()
