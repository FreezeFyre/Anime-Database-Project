#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <variant>

#include "net/protocol/data.hpp"
#include "io/fileHandling.hpp"

#include "databases/entry_data.hpp"
#include "databases/users.hpp"
#include "databases/auth/auth_service.hpp"
#include "filtering/filterProcessor.hpp"
#include "core.hpp"
#include "net/transport/session.hpp"
#include "net/routing/router.hpp"


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




namespace anime::protocol {
using namespace anime::app;
using namespace anime::auth;
using anime::net::Session;
class WebSocketCodec {
	public:
	    AppRequest decode(ByteVector& byteStream, Context& context, Session& session);

		ByteVector encode(const AppRequest& request, const AppResponse& response);

	private:
	    ByteVector encodeHandler(const EntryResponse& response);
	    ByteVector encodeHandler(const AuthResponse& response);

};

} // namespace anime::protocol
