import serial
import constant
import platform

class Arduino:
	def __init__(self):
		i = 0
		host = platform.system()
		print("Attemping to connect... Host is: " + host)

		if (host == "Windows"):
			PATH = 'COM'
		else:
			PATH = '/dev/tty.usbmodem'

		while True:
			try:
				COM = PATH + str(i)
				self.serial = serial.Serial(COM)
				break
			except:
				i += 1

		print("Successfully connected to %s" % COM)

	def resetTime(self):
		self.serial.write(constant.COMMAND_RESET_TIME.encode())
		rxbuffer = listen(self.serial)

		return str(rxbuffer[0])

	def getTime(self):
		self.serial.write(constant.COMMAND_GET_TIME.encode())
		rxbuffer = listen(self.serial)

		j = 0
		num = 0
		for i in range(len(rxbuffer)):
			num += rxbuffer[i] * (2 ** j)
			j += constant.EIGHT_BITS

		print(num)
		return str(num)

	def registerCard(self, id):
		self.serial.write(constant.COMMAND_REGISTER.encode())
		self.serial.write(id.encode())
		
		print("ID sent: " + id)
		rxbuffer = listen(self.serial)

		return str(rxbuffer[0])



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
