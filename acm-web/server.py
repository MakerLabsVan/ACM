import constant
from arduino import Arduino
from flask import Flask, render_template

app = Flask(__name__, static_url_path='/static')
arduino = Arduino()

@app.route("/")
def index():
    return render_template("dashboard.html")

@app.route("/getTime")
def getTime():
	data = arduino.getTime()
	if data == constant.ERROR_TIMEOUT:
		return 0
	else:
		return data
	
@app.route("/resetTime")
def resetTime():
	data = arduino.resetTime()
	if data == constant.ERROR_TIMEOUT:
		return 0
	else:
		return data

if __name__ == "__main__":
    app.run()