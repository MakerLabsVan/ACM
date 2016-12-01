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
		elif database.existingUser(data):
			print("ID updated")
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
	# need to figure out why large ids are being sent randomly
	if 0 < id and id < 50000:
		# ignore logging, resetting and refreshing guest card permissions
		if id not in constant.GUEST_IDS:
			socketio.emit('scan', id)		
			# push data to web app
			data = database.retrieveUser(id)			
			socketio.emit('data', data)		

			# reset card if a month has passed
			if data[-1] == "1":
				resetTime()

			# refresh card permissions
			if refresh(id, data) == str(id):
				refreshStatus = "Card successfully updated"
			else:
				refreshStatus = "Card not updated"

			socketio.emit('refresh', refreshStatus)

			database.scanLog(id)
						
	return str(id)

@app.route("/refresh")
def refresh(id, data):
	userData = [ str(id), data[constant.COL_MEMBER_TYPE] ]
	userData.extend(data[constant.COL_USES_LASER_A:constant.COL_USES_3D+1])
	print(userData)
	return arduino.refreshUser(userData)

if (__name__ == "__main__"):
    # app.run(host='0.0.0.0')
	socketio.run(app, host='0.0.0.0')