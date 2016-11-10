import constant
from database import Database
from arduino import Arduino
from flask import Flask, request, render_template
from flask_socketio import SocketIO, emit

app = Flask(__name__)
socketio = SocketIO(app)
arduino = Arduino()	
database = Database()

@app.route("/")
def index():
    return render_template("dashboardv2.html")

@app.route("/getTime")
def getTime():
	return arduino.getTime()
	
@app.route("/resetTime")
def resetTime():
	return arduino.resetTime()

@app.route("/registerCard", methods=['POST'])
def registerCard():
	data = request.get_json()
	arduinoStatus = arduino.registerCard(data)
	if arduinoStatus == data["uid"]:
		print("RFID tag registered")
		if data["isNew"]:
			return database.insertUser(data)
		else:
			return arduinoStatus
	else:
		return "0"

@app.route("/laserLog/<laser>/<int:id>/<int:elapsedTime>/<int:existingTime>")
def laserLog(laser, id, elapsedTime, existingTime):
	data = {"laserType": laser, "uid": id, "elapsedTime": elapsedTime, "existingTime": existingTime}
	database.laserLog(data)
	return database.insertLaserTime(data)

@app.route("/scanTest/<int:id>")
def serialTest(id):
	if id != 0 and id < 50000:		
		data = database.retrieveUser(id)
		socketio.emit('scan', data)

		# reset card if a month has passed
		if data[-1] == "1":
			resetTime()

		# ignore logging and refreshing guest cards
		if id != 6 and id != 12:
			database.scanLog(id)
			if refresh(id, data) == str(id):
				print("RFID tag refreshed")

	return str(id)

@app.route("/refresh")
def refresh(id, data):
	userData = [ str(id), data[3] ]
	userData.extend(data[constant.COL_USES_LASER_A:constant.COL_USES_3D+1])
	print(userData)
	return arduino.refreshUser(userData)

if (__name__ == "__main__"):
    # app.run(host='0.0.0.0')
	socketio.run(app, host='0.0.0.0')