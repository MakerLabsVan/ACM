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
				i = i + 1

		print("Successfully connected to COM%d" % i)

	def resetCard(self):
		self.serial.write(constant.LED_OFF.encode())

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
		self.serial.write(constant.LED_ON.encode())

		k = 0
		num = 0
		rxbuffer = []
		while True:
			byte = list(self.serial.read())
			if byte[0] == 0:
				break
			else:
				rxbuffer.append(byte[0])

		for j in range(len(rxbuffer)):
			num = num + rxbuffer[j] * (2 ** k)
			k = k + EIGHT_BITS

		print(rxbuffer)
		print(num)
		return str(num)