import serial
import constant

arduino = ''

def beginSerial():
	i = 0
	global arduino

	while True:
		try:
			COM = 'COM' + str(i)
			arduino = serial.Serial(COM, constant.BAUD_RATE)
			break
		except:
			print("COM%d not able to connect. Trying next COM port" % i)
			i = i + 1

	print("Successfully connected to COM%d" % i)
	return arduino

def sendCommand(id):
	command = str(id).encode()
	arduino.write(command)
	numBytesAvailable = arduino.inWaiting()
	print(numBytesAvailable)

	# if numBytesAvailable == 0:
	# 	print(arduino.read(numBytesAvailable))

	print(arduino.read(numBytesAvailable))

