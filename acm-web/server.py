import constant
from database import Database
from arduino import Arduino
from flask import Flask, jsonify, request, render_template
from flask_socketio import SocketIO, emit

app = Flask(__name__)
socketio = SocketIO(app)
arduino = Arduino()
database = Database()

@app.route("/")
def index():
	return render_template("acm.html")

@app.route("/getTime")
def getTime():
	return arduino.getTime()
	
@app.route("/resetTime")
def resetTime():
	return arduino.resetTime()

@app.route("/registerCard", methods=['POST'])
def registerCard():
	data = request.get_json()

	# Register RFID
	arduinoStatus = arduino.registerCard(data)
	if arduinoStatus == data["uid"]:
		print("RFID tag registered")
		# if new to ACM, create a new entry
		if data["isNew"]:
			return database.insertUser(data)
		# modify if not new to ACM
		else:
			database.existingUser(data)
			return arduinoStatus
	else:
		return str(0)

@app.route("/laserLog/<laser>/<int:id>/<int:elapsedTime>/<int:existingTime>")
def laserLog(laser, id, elapsedTime, existingTime):
	data = {"laserType": laser, "uid": id, "elapsedTime": elapsedTime, "existingTime": existingTime}
	database.laserLog(data)
	return database.insertLaserTime(data)

@app.route("/scanTest/<int:id>")
def serialTest(id):
	# need to figure out why large ids are being sent randomly - probably from bad reads
	print("ID: " + str(id));
	if 0 < id and id < 50000:
		# ignore guest cards
		if id not in constant.GUEST_IDS:
			# immediately send ID to web app
			socketio.emit('scan', id)
			print("SCANNED")

			# once data is received, send to web app
			data = database.retrieveUser(id)
			socketio.emit('data', data)

			# send data to arduino
			refresh(id, data)

			database.scanLog(id)
						
	return str(id)

def refresh(id, data):
	# format user data according to card protocol
	userData = [ str(id), data[constant.COL_MEMBER_TYPE] ]
	userData.extend( data[constant.COL_USES_LASER_A:constant.COL_USES_3D+1] )
	return arduino.refreshUser(userData)

@app.route("/laserData/<type>")
def laserData(type):
	data = database.retrieveData(type)
	return jsonify(data)

if (__name__ == "__main__"):
    # app.run(host='0.0.0.0')
	socketio.run(app, host='0.0.0.0')