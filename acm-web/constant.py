# Arduino related
COMMAND_GET_TIME = '1'
COMMAND_RESET_TIME = '2'
COMMAND_REGISTER = '3'
COMMAND_REFRESH ='5'

# User Data related
# These refer to list indices, not columns in the spreadsheet; add 1 to get actual column number
# A new machine would require an additional entry here
CELL_PKEY = 'A1'
COL_PKEY = 0
COL_UID = 1
COL_MEMBER_NAME = 2
COL_MEMBER_TYPE = 3
COL_START_DAY = 4
COL_USES_LASER_A = 9
COL_USES_LASER_B = 10
COL_USES_SHOPBOT = 11
COL_USES_WOOD = 12
COL_USES_METAL = 13
COL_USES_TEXTILE = 14
COL_USES_3D = 15
COL_LIFETIME = 16
COL_LASERTIME = 17
NUM_FROZEN_ROWS = 1
# These refer to actual columns
COL_LASER_TIME = 18
COL_LASER_TIME_START = 'Q'
COL_LASER_TIME_END = ':R'
COL_START_DATA = 'A'
# This will need to be changed if more columns are added
COL_END_DATA = ':R'

# Log related
RANGE_SCAN_START = 'A'
RANGE_SCAN_END = ':C'
RANGE_LASER_START = 'A'
RANGE_LASER_END = ':E'
COL_LASER_TYPE = 0
COL_DATE = 1
COL_ID_LOG = 2
COL_ELAPSED_TIME = 3
COL_EXISTING_TIME = 4

TIME_TOKEN_EXPIRE = 1800
SECONDS_IN_WEEK = 7*24*60*60
SECONDS_IN_MIN = 60
EIGHT_BITS = 8
GUEST_IDS = [ 6, 12 ]