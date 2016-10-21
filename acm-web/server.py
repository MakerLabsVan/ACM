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
	if arduino.registerCard(data) == data["uid"]:
		return database.insertUser(data)
	else:
		return "0"

@app.route("/laserLog/<laser>/<int:id>/<int:elapsedTime>/<int:existingTime>")
def laserLog(laser, id, elapsedTime, existingTime):
	data = {"laserType": laser, "uid": id, "elapsedTime": elapsedTime, "existingTime": existingTime}
	database.laserLog(data)
	return database.insertLaserTime(data)

@app.route("/refresh/<int:id>")
def refresh(id):
	data = database.refreshUser(id)
	return arduino.refreshUser(data)

# ----------------------------------------
@app.route("/scanTest/<int:id>")
def serialTest(id):
	# ignore guest cards for now
	if id != 0 and id != 6 and id != 12:
		database.scanLog(id)
		data = database.retrieveUser(id)
		socketio.emit('scan', data)
	return str(id)

if (__name__ == "__main__"):
    # app.run(host='0.0.0.0')
	socketio.run(app, host='0.0.0.0')