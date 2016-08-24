import serial
import time

i = 0
while True:
	try:
		com = 'COM' + str(i)
		ser = serial.Serial(com, 57600)
		break
	except:
		print("COM%d not able to connect. Trying next COM port" % i)
		i = i + 1

print("Successfully connected to COM%d" % i)
time.sleep(5)

while True:
	command1 = '1'
	print("Turning LED on")
	ser.write(command1.encode())
	time.sleep(2)

	command2 = '2'
	print("Turning LED off")
	ser.write(command2.encode())
	time.sleep(2)