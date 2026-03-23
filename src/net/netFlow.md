Client
   ↓
[ TCP Server ]
   ↓
[ Request Parser (your custom format) ]
   ↓
[ Router ]
   ↓
[ Query / Filter Builder ]
   ↓
[ animeDB ]
   ↓
[ Binary Response Builder ]
   ↓
[ TCP Send ]





Possible Communication Content:

Client --> Server
	Auth:
		Login
		Logout
		Create Account
		Change Password
		Refresh Session
	Queries:
		Search
		Filter
		Get By ID
		Request Sorted IDS
	Mutation/Admin:
		For Group,Anime, or Character
			Add
			Update
			Delete
			Update Lookup Tables
	Utility: 
		Ping
		Request Schema


Server --> Client
	Core:
		Anime Data
		Character Data
		Group Data
	Query Results:
		Filtered IDs
		Search Results(IDs maybe scored)
		Sorted IDs
	MetaData:
		Schema
		Lookup Tables
		PreComputed Sorts
	Auth Responses:
		Success/Failure
		Session Token
	Errors:
		Invalid Request
		Invalid Filters
		Auth Fail
		Version Mismatch





Each data type being sent needs to be serialized into BYTES in order to be sent, 

string = [uint32 length][content_bytes]

vector = [uint32 fieldCount][f1][f2]......[fN]

range = [active_fields][f1][f2]		(active_fields=bitmask	flags=0b01min	flags=0b10max	flags=0b11both)

uint8 = [byte]

uint16 = [byte][byte]

uint32 = [byte][byte][byte][byte]

float32 = [byte][byte][byte][byte]

float64 = [byte][byte][byte][byte][byte][byte][byte][byte]
