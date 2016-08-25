import serial
import constant
from binascii import hexlify, unhexlify

class Arduino:
	def __init__(self):
		i = 0
		while True:
			try:
				COM = 'COM' + str(i)
				self.serial = serial.Serial(COM)
				break
			except:
				print("COM%d not able to connect. Trying next COM port" % i)
				i = i + 1

		print("Successfully connected to COM%d" % i)

	def resetCard(self):
		self.serial.write(constant.LED_OFF.encode())

		rxbuffer = []
		while True:
			byte = self.serial.read()
			if byte == constant.END_CHAR.encode():
				break
			else:
				rxbuffer.append(byte)

		print(rxbuffer)

	def getTime(self):
		self.serial.write(constant.LED_ON.encode())

		k = 0
		num = 0
		rxbuffer = []
		while True:
			byte = list(self.serial.read())
			if byte[0] == 44:
				break
			else:
				rxbuffer.append(byte[0])

		print(rxbuffer)
		for j in range(len(rxbuffer)):
			num = num + rxbuffer[j] * (2 ** k)
			k = k + 8
		print(num)