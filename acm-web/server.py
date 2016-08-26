import constant
from arduino import Arduino
from flask import Flask, render_template

app = Flask(__name__, static_url_path='/static')
arduino = Arduino()

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/on")
def on():
	return arduino.getTime()
	
@app.route("/off")
def off():
	return arduino.resetCard()

if __name__ == "__main__":
    app.run()