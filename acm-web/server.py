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
	status = database.insertUser(data)

	if status == "1":
		return "ID already exists."
	else:
		return str(1)
		#return arduino.registerCard(data)

if __name__ == "__main__":
    app.run()