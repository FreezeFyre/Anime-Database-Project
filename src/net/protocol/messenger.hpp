#pragma once

#include "net/protocol/data.hpp"
#include "protocol/app_messages.hpp"

namespace anime::protocol {

class WebSocketCodec {
	public:
	    AppRequest decode(const ByteVector& byteStream);

		ByteVector encode(const AppRequest& request, const AppResponse& response);

	private:
	    ByteVector encodeHandler(const EntryResponse& response);
	    ByteVector encodeHandler(const AuthResponse& response);

};

} // namespace anime::protocol
