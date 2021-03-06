import serial
import constant
from platform import system

class Arduino:
	# Connect to the Arduino through serial
	# This should work for all platforms
	# Only been tested on Windows and Mac
	def __init__(self):
		i = 0
		host = system()
		print("Attemping to connect... Host is: " + host)

		if (host == "Windows"):
			PATH = 'COM'
		else:
			PATH = '/dev/tty.usbmodem'

		while True:
			try:
				COM = PATH + str(i)

				# exception will occur here
				self.serial = serial.Serial(COM)
				print("Successfully connected to %s" % COM)				
				break
			except:
				i += 1
				if i == 1500:
					print("Scanner not detected, trying to connect on" + ": " + COM)
					break
		

	def resetTime(self):
		self.serial.write(constant.COMMAND_RESET_TIME.encode())
		rxbuffer = listen(self.serial)

		return str(rxbuffer[0])

	def getTime(self):
		self.serial.write(constant.COMMAND_GET_TIME.encode())
		rxbuffer = listen(self.serial)
		
		return bytesToNum(rxbuffer)

	# This only writes data up to the Laser A. For future expansion, 
	# add more lines here for each additional machine.
	def registerCard(self, data):
		self.serial.write(constant.COMMAND_REGISTER.encode())
		# send the number of digits, so that Arduino can reconstruct the ID from ASCII characters
		self.serial.write( str(len(data["uid"])).encode() )
		self.serial.write(data["uid"].encode())
		self.serial.write(data["memberType"].encode())
		self.serial.write(data["laserA"].encode())

		rxbuffer = listen(self.serial)

		return bytesToNum(rxbuffer)
	
	def refreshUser(self, data):
		self.serial.write(constant.COMMAND_REFRESH.encode())
		self.serial.write( str(len(data[0])).encode() )
		for i in range(len(data)):
			self.serial.write(data[i].encode())
		
		rxbuffer = listen(self.serial)

		return bytesToNum(rxbuffer)

# waits for serial data until the end character is reached
def listen(arduino):
	rxbuffer = []
	while True:
		byte = list(arduino.read())

		if byte[0] == 0:
			break
		else:
			rxbuffer.append(byte[0])
	
	print(rxbuffer)
	return rxbuffer

# converts an array of bytes to a single number
# serial data from Arduino is sent by the lower 8 bits first
# so the number "reads" from right to left in the buffer
def bytesToNum(rxbuffer):
	j = 0
	num = 0
	for i in range(len(rxbuffer)):
		num += rxbuffer[i] * (2 ** j)
		j += constant.EIGHT_BITS
	
	print(num)
	return str(num)
