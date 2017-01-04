import gspread
import constant
from datetime import datetime, timedelta
from oauth2client.service_account import ServiceAccountCredentials

class Database:
    def __init__(self):
        self.authorize(isStartUp=True)
    
    # authorizes and opens Google Sheet
    def authorize(self, isStartUp=False):
        if isStartUp or self.isTokenExpired():
            scope = ['https://spreadsheets.google.com/feeds']
            credentials = ServiceAccountCredentials.from_json_keyfile_name('../../acm-dashboard.json', scope)
            gc = gspread.authorize(credentials)
            self.lastAuthorizationTime = datetime.now()
            print("Authorization complete at time: " + str(self.lastAuthorizationTime.time().replace(microsecond=0)))

            spreadsheet = gc.open("MakerLabs ACM")
            self.user_data = spreadsheet.worksheet("Users")
            self.scan_data = spreadsheet.worksheet("Check Ins")
            self.laser_data = { "A": spreadsheet.worksheet("Laser A Log"), "B": spreadsheet.worksheet("Laser B Log") }

            print("Opened database")
    
    # helper that determines if Google API access token has expired
    def isTokenExpired(self):
        currentTime = datetime.now()
        referenceTime = self.lastAuthorizationTime
        difference = currentTime - referenceTime

        if difference.total_seconds() > constant.TIME_TOKEN_EXPIRE:
            return True
        else:
            return False

    def insertUser(self, data):
        print("Inserting " + data["memberName"])
        self.authorize()
        self.pKey = int(self.user_data.acell(constant.CELL_PKEY).value)        
        
        # Create new row
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
        # The leading apostrophe is to prevent Google Sheets from autoformatting date string
        # Slice timestamp string to only input the date
        cellList[constant.COL_START_DAY].value = "'" + data["startDay"][0:10]
        cellList[constant.COL_USES_LASER_A].value = data["laserA"]
        cellList[constant.COL_USES_LASER_B].value = data["laserB"]
        cellList[constant.COL_USES_SHOPBOT].value = data["shopbot"]
        cellList[constant.COL_USES_WOOD].value = data["wood"]
        cellList[constant.COL_USES_METAL].value = data["metal"]
        cellList[constant.COL_USES_TEXTILE].value = data["textile"]
        cellList[constant.COL_USES_3D].value = data["threeD"]
        cellList[constant.COL_LIFETIME].value = "1"
        cellList[constant.COL_LASERTIME].value = "1"

        # Increment primary key
        self.pKey += 1
        cellList[-1].value = self.pKey
        
        # Push changes to sheet
        self.user_data.update_cells(cellList)
        return data["uid"]
    
    # replace an existing users RFID
    def existingUser(self, data):
        self.authorize()
        try:
            user = self.user_data.find(data["memberName"])
            self.user_data.update_cell(user.row, user.col - 1, data["uid"])
            print("ID updated")
            return True
        except:
            return False
    
    def scanLog(self, id):
        print("Logging User %d at Front Desk" % id)
        
        self.authorize()
        currentRow = int(self.scan_data.acell(constant.CELL_PKEY).value) + 1
        
        cellList = []
        cellList.append(currentRow)
        cellList.append(datetime.now().date().isoformat() + " " + datetime.now().time().isoformat())
        cellList.append(id)

        self.scan_data.append_row(cellList)
        self.scan_data.update_acell(constant.CELL_PKEY, currentRow)
    
    def laserLog(self, data):
        print("Logging user %d on Laser %s" % (data["uid"], data["laserType"]))

        self.authorize()
        currentRow = int(self.laser_data[ data["laserType"] ].acell(constant.CELL_PKEY).value) + 1
        
        cellList = []
        cellList.append(currentRow)
        cellList.append(datetime.now().date().isoformat() + " " + datetime.now().time().isoformat())
        cellList.append(data["uid"])
        cellList.append(data["elapsedTime"])
        cellList.append(data["existingTime"])

        self.laser_data[ data["laserType"] ].append_row(cellList)
        self.laser_data[ data["laserType"] ].update_acell(constant.CELL_PKEY, currentRow)
    
    # add to accumulated time on Users sheet
    def insertLaserTime(self, data):
        self.authorize()
        userRow = self.searchID(data["uid"])

        # Update times
        if userRow:
            cellList = self.user_data.range(constant.COL_LASER_TIME_START + str(userRow) + constant.COL_LASER_TIME_END + str(userRow))
            cellList[0].value = int(cellList[0].value) + data["elapsedTime"]
            cellList[1].value = int(cellList[1].value) + data["elapsedTime"]
            self.user_data.update_cells(cellList)
            print("Updated user at row %d" % userRow)
            
        
        return str(userRow)
    
    # return the row of the user
    def searchID(self, id):
        idList = self.user_data.col_values(constant.COL_MEMBER_NAME)

        try:
            row = idList.index(str(id)) + 1
        except:
            row = 0
            print("User does not exist")            

        return row 
    
    # return all user information
    def retrieveUser(self, id):
        print("Getting data for user %d" % id)
        self.authorize()
        userRow = self.searchID(id)
        
        # If ID found, retrieve row and all values related to machine authorizations
        if userRow:
            userData = []
            userCells = self.user_data.range(constant.COL_START_DATA + str(userRow) + constant.COL_END_DATA + str(userRow))
            
            # Create list of cell values to pass
            for cell in userCells:
                userData.append(cell.value)
            
            print(userData)
            return userData

        # If not found, return 0
        else:
            return str(userRow)
    
    def getName(self, log):
        # Google Sheets indexes starting at 1
        userIDs = self.user_data.col_values(constant.COL_UID + 1)
        memberNames = self.user_data.col_values(constant.COL_MEMBER_NAME + 1)

        # go through each log entry
        for entry in log:
            # get current id
            id = entry[constant.COL_UID]
            # find corresponding row
            try:
                row = userIDs.index(id)
                # replace with name
                entry[constant.COL_UID] = memberNames[row]
            except:
                entry[constant.COL_UID] += " - Unknown user!"
            
        return log
    
    # laser data retrieval
    def retrieveData(self, type):
        self.authorize()
        
        consolidatedLogs = []
        data = self.laser_data[type].get_all_values()

        # sort in descending order
        cell = sorted(data, key = lambda x: x[1], reverse = True)
        newLog = [ cell[1][constant.COL_DATE], cell[1][constant.COL_ID_LOG], int(cell[1][constant.COL_ELAPSED_TIME]) ]
        
        for row in range(2, len(cell)):
            # log is more than a week old
            # slice the timestamp string to only take in the date portion
            if not self.isWeek(newLog[0][0:10]):
                newLog[2] = int(newLog[2] / constant.SECONDS_IN_MIN)
                consolidatedLogs.append(newLog) 
                break
            # if IDs are the same, accumulate the time
            if newLog[1] == cell[row][constant.COL_ID_LOG]:
                newLog[2] += int(cell[row][constant.COL_ELAPSED_TIME])
                # end of spreadsheet is reached
                if row == len(cell) - 1:
                    newLog[2] = int(newLog[2] / constant.SECONDS_IN_MIN)
                    consolidatedLogs.append(newLog)
            # new ID detected, create log and start a new one
            else:
                newLog[2] = int(newLog[2] / constant.SECONDS_IN_MIN)
                consolidatedLogs.append(newLog)
                newLog = [ cell[row][constant.COL_DATE], cell[row][constant.COL_ID_LOG], int(cell[row][constant.COL_ELAPSED_TIME]) ]

        return self.getName(consolidatedLogs)
    
    # helper that takes in a date string and determines if a week has passed
    def isWeek(self, referenceDate):
        currentDate = datetime.now()
        referenceDate = datetime.strptime(referenceDate, "%Y-%m-%d")
        difference = currentDate - referenceDate

        if difference.total_seconds() < constant.SECONDS_IN_WEEK:
            return True
        else:
            return False