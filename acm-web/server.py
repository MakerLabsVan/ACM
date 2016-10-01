import constant
from database import Database
from arduino import Arduino
from flask import Flask, request, render_template, json

app = Flask(__name__)
#arduino = Arduino()
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
	
	#return data["uid"]
	return arduino.registerCard(data)

@app.route("/laserLog/<int:id>/<int:elapsedTime>/<int:existingTime>")
def laserLog(id, elapsedTime, existingTime):
	print("Logging user %d" % id)
	data = json.dumps({"id": id, "elapsedTime": elapsedTime, "existingTime": existingTime})
	database.insertLaserTime(data)
	return "1"

@app.route("/serialTest/<int:id>")
def serialTest(id):
	print("ayy from arduino, id sent: %d" % id)
	return "1"

if (__name__ == "__main__"):
    app.run(host='0.0.0.0')