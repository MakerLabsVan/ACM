import gspread
import constant
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
        self.user_data.add_rows(1)
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
        cellList[constant.COL_USES_THREE_D].value = data["threeD"]

        # Increment primary key
        self.pKey += 1
        cellList[-1].value = self.pKey
        
        # Push changes to sheet
        self.user_data.update_cells(cellList)
        print("Done")
    
    def insertLaserTime(self, data):
        print(data)
        self.laser_data.update_acell('A1', "ayy")