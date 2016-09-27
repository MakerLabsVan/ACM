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
        self.sheet = spreadsheet.worksheet("Users")
        self.pKey = int(self.sheet.acell('A1').value)
        print("Opened database")
    
    def insertUser(self, data):
        print("Inserting " + data["memberName"])

        # Create new row and get that row as a list of cell objects
        self.sheet.add_rows(1)
        endRow = self.sheet.row_count
        cellList = self.sheet.range(constant.COL_START_DATA + str(endRow) + constant.COL_END_DATA + str(endRow))

        # Update cell values
        cellList[constant.COL_PKEY].value = self.pKey
        cellList[constant.COL_UID].value = data["id"]
        cellList[constant.COL_MEMBER_NAME].value = data["memberName"]
        cellList[constant.COL_MEMBER_TYPE].value = data["memberType"]
        cellList[constant.COL_USES_LASER_A].value = data["laserA"]
        cellList[constant.COL_USES_LASER_B].value = data["laserB"]
        cellList[constant.COL_USES_SHOPBOT].value = data["shopbot"]
        cellList[constant.COL_USES_WOOD].value = data["wood"]
        cellList[constant.COL_USES_METAL].value = data["metal"]
        cellList[constant.COL_USES_TEXTILE].value = data["textile"]
        cellList[constant.COL_USES_THREE_D].value = data["threeD"]

        # Push to sheet and increment primary key
        self.pKey += 1
        self.sheet.update_acell(constant.CELL_PKEY, self.pKey)
        self.sheet.update_cells(cellList)