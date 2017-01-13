# ACM Web Application  
This is a locally run application that serves as a web interface for ACM data and controlling the database. An RFID scanner
is attached through a serial connection to the application. This app can run on Windows, OS X and Linux.

# Requirements  
Flask (http://flask.pocoo.org)  
* SocketIO extension (https://flask-socketio.readthedocs.io/en/latest/)  

AngularJS  
* SocketIO (https://socket.io)  

Python 3 (https://www.python.org/downloads/) 
* PySerial (https://pythonhosted.org/pyserial/)  
* GSpread (https://gspread.readthedocs.io/en/latest/)  

# Installation  
1. Install Python 3 and Flask
2. Install PySerial, GSpread, and Flask-SocketIO modules
3. Follow OAuth2 procedure in GSpread documentation UNLESS still using original spreadsheet.  
   You will still need to install oauth2client `pip install --upgrade oauth2client` and PyOpenSSL `pip install PyOpenSSL`    
   Email Brian for authorization file. See OAuth2 procedure for details on how authorization is obtained.    
   In the database.py file, adjust the relative path to the JSON keyfile if needed.  
   `credentials = ServiceAccountCredentials.from_json_keyfile_name('../../acm-dashboard.json', scope)`
4. App is started by running `python3 server.py`

# Notes
* Photos are uploaded to amazon s3. Photo naming scheme is "id".jpg
