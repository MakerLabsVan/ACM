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
        print("Inserting " + data["name"])

        id_list = self.sheet.col_values(1)

        # Check if ID has already been registered
        for i in range(len(id_list)):
            if id_list[i] == data["id"]:
                print("ID exists. Exiting...")
                return constant.NOT_COMPLETED
            elif id_list[i] == '':
                new_row = i + 1
                break
        
        # Put data in sheet
        # WTF json/list is randomized every time... can't rely on a list as of right now :(
        self.sheet.update_cell(new_row, 1, data["id"])
        self.sheet.update_cell(new_row, 2, data["name"])
        self.sheet.update_cell(new_row, 3, data["type"])
        self.sheet.update_cell(new_row, 4, data["laser"])

        return constant.COMPLETED
    
    def deleteUser(self, data):
        # Determine if deleting by User ID or by Name
        if data["id"] != '':
            col = 1
            key = "id"
        elif data["name"] != '':
            col = 2
            key = "name"
        else:
            return "Invalid choice"
        
        # Pick correct column in sheet
        data_list = self.sheet.col_values(col)

        # Look for value, return if not found
        for i in range(len(data_list)):
            if data_list[i] == data[key]:
                break
            elif data_list[i] == '':
                print("User not found.")
                return constant.NOT_COMPLETED
        
        # If found, replace row with blanks
        for j in range(len(data)):
            self.sheet.update_cell(i+1, j+1, "")
        
        return constant.COMPLETED
                