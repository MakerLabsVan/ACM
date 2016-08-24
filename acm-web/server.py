import initport
from flask import Flask
from flask import request, render_template

app = Flask(__name__, static_url_path='/static')

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/serial")
def serial():
	initport.beginSerial()
	return render_template("index.html")

if __name__ == "__main__":
    app.run()