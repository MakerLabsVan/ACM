import commands
import constant
from arduino import Arduino
from flask import Flask
from flask import request, render_template

app = Flask(__name__, static_url_path='/static')
arduino = Arduino()

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/on")
def on():
	arduino.sendCommand(constant.LED_ON)
	return render_template("index.html")

@app.route("/off")
def off():
	arduino.sendCommand(constant.LED_OFF)
	return render_template("index.html")

if __name__ == "__main__":
    app.run()