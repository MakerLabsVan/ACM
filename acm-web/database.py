import gspread
import constant
from datetime import datetime
from oauth2client.service_account import ServiceAccountCredentials

class Database:
    def __init__(self):
        scope = ['https://spreadsheets.google.com/feeds']
        credentials = ServiceAccountCredentials.from_json_keyfile_name('../../acm-dashboard.json', scope)
        gc = gspread.authorize(credentials)
        print("Authorization complete")

        spreadsheet = gc.open("MakerLabs ACM")
        self.user_data = spreadsheet.worksheet("Users")
        self.laser_data = spreadsheet.worksheet("Laser Log")
        self.pKey = int(self.user_data.acell(constant.CELL_PKEY).value)
        print("Opened database")
    
    def insertUser(self, data):
        print("Inserting " + data["memberName"])

        # Create new row and get that row as a list of cell objects
        prevRowCount = self.user_data.row_count
        currRowCount = prevRowCount
        while currRowCount == prevRowCount:
            self.user_data.add_rows(1)
            currRowCount = self.user_data.row_count

        endRow = self.user_data.row_count
        cellList = self.user_data.range(constant.COL_START_DATA + str(endRow) + constant.COL_END_DATA + str(endRow))
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
    
    def laserLog(self, data):
        print("Logging user %d on Laser %s" % (data["id"], data["laserType"]))

        # Add and retrieve new row
        prevRowCount = self.laser_data.row_count
        currRowCount = prevRowCount
        while currRowCount == prevRowCount:
            self.laser_data.add_rows(1)
            currRowCount = self.laser_data.row_count

        endRow = self.laser_data.row_count
        cellList = self.laser_data.range(constant.RANGE_LASER_START + str(endRow) + constant.RANGE_LASER_END + str(endRow))

        # Insert and push updated cells to sheet
        cellList[constant.COL_LASER_TYPE].value = data["laserType"]
        cellList[constant.COL_DATE].value = datetime.now().date().isoformat()
        cellList[constant.COL_TIME].value = datetime.now().time().isoformat()
        cellList[constant.COL_ID_LOG].value = data["id"]
        cellList[constant.COL_ELAPSED_TIME].value = data["elapsedTime"]
        cellList[constant.COL_EXISTING_TIME].value = data["existingTime"]
        self.laser_data.update_cells(cellList)
    
    def insertLaserTime(self, data):
        # Search for row containing user ID ** NOT CONSTANT TIME
        userRow = self.searchID(data["id"])

        # Update times
        if userRow:
            cellList = self.user_data.range(constant.COL_LASER_TIME_START + str(userRow) + constant.COL_LASER_TIME_END + str(userRow))
            cellList[0].value = int(cellList[0].value) + data["elapsedTime"]
            cellList[1].value = int(cellList[1].value) + data["elapsedTime"]
            self.user_data.update_cells(cellList)
        
        return str(userRow)

    
    def refreshUser(self, id):
        print("Getting data for user %d" % id)

        # Search for row containing user ID ** NOT CONSTANT TIME
        userRow = self.searchID(id)
        
        # If ID found, retrieve row and all values related to machine authorizations
        if userRow:
            userData = [str(id)]
            userCells = self.user_data.range(constant.COL_START_DATA + str(userRow) + constant.COL_END_DATA + str(userRow))
            
            # Create list of cell values to pass to Arduino
            userData.append(userCells[constant.COL_MEMBER_TYPE].value)
            for i in range(constant.COL_USES_LASER_A, constant.COL_USES_3D + 1):
                userData.append(userCells[i].value)
            
            print(userData)
            return userData

        # If not found, return 0
        else:
            return str(userRow)
        
    def searchID(self, id):
        row = 0
        idList = self.user_data.col_values(constant.COL_MEMBER_NAME)

        for i in range(len(idList)):
            if idList[i] == "" or idList[i] == "RFID#":
                continue
            elif int(idList[i]) == id:
                row = i + 1
                break
        
        return row