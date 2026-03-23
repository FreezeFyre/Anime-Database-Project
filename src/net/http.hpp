#include <string>
#include <unordered_map>

struct Request {
	uint8_t COM_TYPE;
	std::vector<uint8_t> payload;
};


struct Response {
	uint8_t COM_TYPE;
	std::vector<uint8_t> payload;
};


static const uint8_t ENCODE_VERSION = 1001;


enum class COM_TYPE : uint8_t {
	AUTH = 1,
	QUERY = 2,
	DATA = 3,
	MUTATION = 4,
	META = 5,
	ERROR = 6
};


enum class QUERY_TYPE : uint8_t {
	SEARCH = 1,
	FILTER = 2,
	GET_BY_ID = 3,
	SORT_REQUEST = 4
};


/*

REQUEST FORMAT


[ENCODE_VERSION]	[COM_TYPE]	[QUERY_TYPE]	[PAYLOAD_SIZE]

[PAYLOAD]


*/

class ENCODE {


};