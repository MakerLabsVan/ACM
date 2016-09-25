import gspread
from oauth2client.service_account import ServiceAccountCredentials

def authorize():
    scope = ['https://spreadsheets.google.com/feeds']
    credentials = ServiceAccountCredentials.from_json_keyfile_name('../../acm-dashboard.json', scope)

    gc = gspread.authorize(credentials)
    print("Authorization complete")
    spreadsheet = gc.open("MakerLabs ACM")
    print("Opened database")
    database = spreadsheet.worksheet("Users")
    return database

def getUsers(database):
    print("Getting all users...\n")
    id_list = database.col_values(1)
    name_list = database.col_values(2)

    for i in range(len(id_list)):
        if id_list[i] != '':
            print(id_list[i] + " " + name_list[i])

    print("\nDone\n")


