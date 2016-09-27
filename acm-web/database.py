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
        print("Opened database")
        self.sheet = spreadsheet.worksheet("Users")
        self.pKey = int(self.sheet.acell('A1').value)

    def getUsers(self):
        print("Getting all users...\n")
        id_list = self.sheet.col_values(1)
        name_list = self.sheet.col_values(2)

        for i in range(len(id_list)):
            if id_list[i] != '':
                print(id_list[i] + " " + name_list[i])
            else:
                break

        print("\nDone\n")
    
    def insertUser(self, data):
        print("Inserting " + data["memberName"])

        self.sheet.add_rows(1)
        endRow = self.sheet.row_count
        cellList = self.sheet.range('A' + str(endRow) + ":M" + str(endRow))

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

        self.pKey += 1
        self.sheet.update_cells(cellList)
        self.sheet.update_acell(constant.CELL_PKEY, self.pKey)