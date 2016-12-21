# ACM Web Application  
This is a locally run application that serves as a web interface for ACM data and controlling the database. An RFID scanner
is attached through a serial connection to the application. This app can run on Windows, OS X and Linux.

# Requirements  
Flask (http://flask.pocoo.org)  
..* SocketIO extension (https://flask-socketio.readthedocs.io/en/latest/)  

AngularJS  
..* SocketIO (https://socket.io)  

Python 3 (https://www.python.org/downloads/) 
..* PySerial (https://pythonhosted.org/pyserial/)  
..* GSpread (https://gspread.readthedocs.io/en/latest/)  

# Installation  
1. Install Python 3 and Flask
2. Install PySerial, GSpread, and Flask-SocketIO modules
3. App is started by entering "python3 server.py"