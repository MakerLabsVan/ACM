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
        self.db = spreadsheet.worksheet("Users")

    def getUsers(self):
        print("Getting all users...\n")
        id_list = self.db.col_values(1)
        name_list = self.db.col_values(2)

        for i in range(len(id_list)):
            if id_list[i] != '':
                print(id_list[i] + " " + name_list[i])

        print("\nDone\n")


