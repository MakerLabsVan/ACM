import commands
from flask import Flask
from flask import request, render_template

app = Flask(__name__, static_url_path='/static')
arduino = commands.beginSerial()

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/on")
def on():
	commands.sendCommand(arduino, 1)
	return render_template("index.html")

@app.route("/off")
def off():
	commands.sendCommand(arduino, 2)
	return render_template("index.html")

if __name__ == "__main__":
    app.run()