import constant
from arduino import Arduino
from flask import Flask, render_template

app = Flask(__name__, static_url_path='/static')
arduino = Arduino()

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/getTime")
def getTime():
	return arduino.getTime()
	
@app.route("/resetTime")
def resetTime():
	return arduino.resetTime()

if __name__ == "__main__":
    app.run()