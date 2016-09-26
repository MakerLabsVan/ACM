import gspread
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
        status = "0"
        print("Inserting " + data["name"])

        id_list = self.sheet.col_values(1)

        for i in range(len(id_list)):
            if id_list[i] == '':
                break
            elif id_list[i] == data["id"]:
                print("ID exists. Exiting...")
                return "1"

        # WTF json/list is randomized every time... can't rely on a list as of right now :(
        self.sheet.update_cell(i, 1, data["id"])
        self.sheet.update_cell(i, 2, data["name"])
        self.sheet.update_cell(i, 3, data["type"])
        self.sheet.update_cell(i, 4, data["laser"])

        return "0"
    
    def deleteUser(self, data):
        # Determine if deleting by User ID or by Name
        if data["id"] != "":
            col = 1
            key = "id"
        elif data["name"] != "":
            col = 2
            key = "name"
        else:
            return "Invalid choice"
        
        # Pick correct column in sheet
        data_list = self.sheet.col_values(col)

        # Look for value, return if not found
        for i in range(len(data_list)):
            if data_list == data[key]
                break
            elif data_list == '':
                print("User not found.")
                return "1"
        
        # If found, replace row with blanks
        for j in range(len(data)):
            self.sheet.update_cell(i, j, "")
                