import constant
from database import Database
from arduino import Arduino
from flask import Flask, request, render_template
from flask_restful import Resource, Api

app = Flask(__name__)
api = Api(app)
#arduino = Arduino()
#database = Database()

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

@app.route("/serialTest")
def serialTest():
	print("hello from arduino wifi")
	return "yay"
	
class HelloWorld(Resource):
	def get(self):
		return {'hello': 'world'}
api.add_resource(HelloWorld, '/helloWorld')

if (__name__ == "__main__"):
    app.run(host='0.0.0.0')