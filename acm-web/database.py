import gspread
import constant
from datetime import datetime, timedelta
from oauth2client.service_account import ServiceAccountCredentials

class Database:
    def __init__(self):
        self.authorize(isStartUp=True)
    
    def authorize(self, isStartUp=False):
        if isStartUp or self.isTokenExpired():
            scope = ['https://spreadsheets.google.com/feeds']
            credentials = ServiceAccountCredentials.from_json_keyfile_name('../../acm-dashboard.json', scope)
            gc = gspread.authorize(credentials)
            self.lastAuthorizationTime = datetime.now()
            print("Authorization complete at time: " + str(self.lastAuthorizationTime.time().replace(microsecond=0)))

            spreadsheet = gc.open("MakerLabs ACM")
            self.user_data = spreadsheet.worksheet("Users")
            self.scan_data = spreadsheet.worksheet("Scan Log")
            self.laser_data = spreadsheet.worksheet("Laser Log")
            self.pKey = int(self.user_data.acell(constant.CELL_PKEY).value)        
            print("Opened database")
    
    def isTokenExpired(self):
        currentTime = datetime.now()
        referenceTime = self.lastAuthorizationTime
        currentTimeDelta = timedelta(days=currentTime.day, hours=currentTime.hour, minutes=currentTime.minute, seconds=currentTime.second).total_seconds()        
        referenceTimeDelta = timedelta(days=referenceTime.day, hours=referenceTime.hour, minutes=referenceTime.minute, seconds=referenceTime.second).total_seconds()

        if (currentTimeDelta - referenceTimeDelta) > constant.TIME_TOKEN_EXPIRE:
            return True
        else:
            return False

    def insertUser(self, data):
        print("Inserting " + data["memberName"])
        self.authorize()

        # Create new row (and make sure its created)
        self.user_data.add_rows(1)
        currRowCount = self.user_data.row_count
        
        # Get that row as a list of cell objects
        cellList = self.user_data.range(constant.COL_START_DATA + str(currRowCount) + constant.COL_END_DATA + str(currRowCount))
        cellList.append(self.user_data.acell(constant.CELL_PKEY))

        # Update cell values
        cellList[constant.COL_PKEY].value = self.pKey
        cellList[constant.COL_UID].value = data["uid"]
        cellList[constant.COL_MEMBER_NAME].value = data["memberName"]
        cellList[constant.COL_MEMBER_TYPE].value = data["memberType"]
        cellList[constant.COL_START_DAY].value = data["startDay"]
        cellList[constant.COL_USES_LASER_A].value = data["laserA"]
        cellList[constant.COL_USES_LASER_B].value = data["laserB"]
        cellList[constant.COL_USES_SHOPBOT].value = data["shopbot"]
        cellList[constant.COL_USES_WOOD].value = data["wood"]
        cellList[constant.COL_USES_METAL].value = data["metal"]
        cellList[constant.COL_USES_TEXTILE].value = data["textile"]
        cellList[constant.COL_USES_3D].value = data["threeD"]

        # Increment primary key
        self.pKey += 1
        cellList[-1].value = self.pKey
        
        # Push changes to sheet
        self.user_data.update_cells(cellList)
        return data["uid"]
    
    def scanLog(self, id):
        print("Logging user %d at Front Desk" % id)
        cellList = [ datetime.now().date().isoformat(), datetime.now().time().isoformat(), id ]
        self.authorize()
        self.scan_data.insert_row(cellList, 2)
    
    def laserLog(self, data):
        print("Logging user %d on Laser %s" % (data["uid"], data["laserType"]))
        self.authorize()

        # Add and retrieve new row
        self.laser_data.add_rows(1)
        currRowCount = self.laser_data.row_count

        cellList = self.laser_data.range(constant.RANGE_LASER_START + str(currRowCount) + constant.RANGE_LASER_END + str(currRowCount))

        # Insert and push updated cells to sheet
        cellList[constant.COL_LASER_TYPE].value = data["laserType"]
        cellList[constant.COL_DATE].value = datetime.now().date().isoformat()
        cellList[constant.COL_TIME].value = datetime.now().time().isoformat()
        cellList[constant.COL_ID_LOG].value = data["uid"]
        cellList[constant.COL_ELAPSED_TIME].value = data["elapsedTime"]
        cellList[constant.COL_EXISTING_TIME].value = data["existingTime"]
        self.laser_data.update_cells(cellList)
    
    def insertLaserTime(self, data):
        self.authorize()
        userRow = self.searchID(data["uid"])

        # Update times
        if userRow:
            cellList = self.user_data.range(constant.COL_LASER_TIME_START + str(userRow) + constant.COL_LASER_TIME_END + str(userRow))
            cellList[0].value = int(cellList[0].value) + data["elapsedTime"]
            cellList[1].value = int(cellList[1].value) + data["elapsedTime"]
            self.user_data.update_cells(cellList)
        
        return str(userRow)
  
    def searchID(self, id):
        idList = self.user_data.col_values(constant.COL_MEMBER_NAME)

        try:
            row = idList.index(str(id)) + 1
        except:
            row = 0

        return row 
# ------------------------------- NOT IN USE -----------------------------------------
    def refreshUser(self, id):
        print("Getting data for user %d" % id)
        self.authorize()

        userRow = self.searchID(id)
        
        # If ID found, retrieve row and all values related to machine authorizations
        if userRow:
            userData = [str(id)]
            userCells = self.user_data.range(constant.COL_START_DATA + str(userRow) + constant.COL_END_DATA + str(userRow))
            
            # Create list of cell values to pass to Arduino
            userData.append(userCells[constant.COL_MEMBER_TYPE].value)
            for i in range(constant.COL_USES_LASER_A, constant.COL_LASER_TIME):
                userData.append(userCells[i].value)
            
            print(userData)
            return userData

        # If not found, return 0
        else:
            print("User does not exist")
            return str(userRow)
    
    def laserLog2(self, data): # this takes 5 - 10 seconds longer than the old append method
        print("Logging user %d on Laser %s" % (data["id"], data["laserType"]))

        cellList = []
        cellList.append( data["laserType"] )
        cellList.append( datetime.now().date().isoformat() )
        cellList.append( datetime.now().time().isoformat() )
        cellList.append( data["id"] )
        cellList.append( data["elapsedTime"] )
        cellList.append( data["existingTime"] )

        self.authorize()
        self.laser_data(cellList, 2)
