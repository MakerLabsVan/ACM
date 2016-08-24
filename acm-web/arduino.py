import serial
import constant

class Arduino:
	def __init__(self):
		i = 0
		while True:
			try:
				COM = 'COM' + str(i)
				self.serial = serial.Serial(COM, constant.BAUD_RATE)
				break
			except:
				print("COM%d not able to connect. Trying next COM port" % i)
				i = i + 1

		print("Successfully connected to COM%d" % i)

	def sendCommand(self, id):
		command = str(id).encode()
		self.serial.write(command)
		numBytesAvailable = self.serial.inWaiting()
		print(numBytesAvailable)

		# if numBytesAvailable == 0:
		# 	print(self.serial.read(numBytesAvailable))

		print(self.serial.read(numBytesAvailable))

