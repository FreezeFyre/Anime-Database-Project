#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <variant>

#include "data.hpp"
#include "../../io/fileHandling.hpp"

#include "../../databases/entry_data.hpp"
#include "../../databases/auth/auth_service.hpp"
#include "../../filtering/filterProcessor.hpp"
#include "../../core.hpp"
#include "../../databases/users.hpp"
#include "../transport/session.hpp"
#include "../routing/router.hpp"


/*

REQUEST FORMAT


[ENCODE_VERSION]
[COM_TYPE]
[DOMAIN]
[ACTION]
[ID]
[PAYLOAD_SIZE]
[PAYLOAD]


*/




class WebSocketCodec {
	public:
	    AppRequest decode(ByteVector& byteStream, Context& context, Session& session);

		ByteVector encode(const AppRequest& request, const AppResponse& response);

	private:
	    ByteVector encodeHandler(const EntryResponse& response);
};
