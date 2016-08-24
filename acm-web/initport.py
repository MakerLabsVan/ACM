import serial

def beginSerial():
	i = 0
	while True:
		try:
			com = 'COM' + str(i)
			arduino = serial.Serial(com, 57600)
			break
		except:
			print("COM%d not able to connect. Trying next COM port" % i)
			i = i + 1

	print("Successfully connected to COM%d" % i)