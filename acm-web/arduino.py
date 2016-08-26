import serial
import constant

class Arduino:
	def __init__(self):
		i = 0
		while True:
			try:
				COM = 'COM' + str(i)
				self.serial = serial.Serial(COM)
				break
			except:
				i += 1

		print("Successfully connected to COM%d" % i)

	def resetTime(self):
		self.serial.write(constant.COMMAND_RESET_TIME.encode())

		rxbuffer = []
		while True:
			byte = list(self.serial.read())
			if byte[0] == 0:
				break
			else:
				rxbuffer.append(byte[0])

		print(rxbuffer)

		return str(rxbuffer[0])

	def getTime(self):
		self.serial.write(constant.COMMAND_GET_TIME.encode())

		k = 0
		num = 0
		rxbuffer = []
		while True:
			byte = list(self.serial.read())
			if byte[0] == 0:
				break
			else:
				rxbuffer.append(byte[0])

		print(rxbuffer)

		for j in range(len(rxbuffer)):
			num = num + rxbuffer[j] * (2 ** k)
			k += EIGHT_BITS

		print(num)
		return str(num)


def listen(arduino, rxbuffer):
	while True:
		byte = list(arduino.read())
		if byte[0] == 0:
			break
		else:
			rxbuffer.append(byte[0])

	print(rxbuffer)

