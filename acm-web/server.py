import constant
from database import Database
from arduino import Arduino
from flask import Flask, request, render_template

app = Flask(__name__)
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
	database.insertUser(data)
	return arduino.registerCard(data)

@app.route("/laserLog/<laser>/<int:id>/<int:elapsedTime>/<int:existingTime>")
def laserLog(laser, id, elapsedTime, existingTime):
	data = {"laserType": laser, "id": id, "elapsedTime": elapsedTime, "existingTime": existingTime}
	database.insertLaserTime(data)
	return "1"

@app.route("/serialTest/<int:id>")
def serialTest(id):
	print("ayy from arduino, id sent: %d" % id)
	return "1"

if (__name__ == "__main__"):
    app.run(host='0.0.0.0')
