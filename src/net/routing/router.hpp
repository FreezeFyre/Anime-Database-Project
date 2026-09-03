#pragma once

#include "protocol/app_messages.hpp"
#include "net/transport/session.hpp"

namespace anime::app {
struct Context;
using anime::protocol::AuthRequest;
using anime::protocol::EntryRequest;
using anime::protocol::AppRequest;
using anime::protocol::AppResponse;
using anime::net::Session;

class Router {
    public:
	    AppResponse route(AppRequest& request, Context& context, Session& session);

	private:
	    AppResponse handle(const EntryRequest& request, uint32_t id, Context& context, Session& session);
		AppResponse handle(const AuthRequest& request, uint32_t id, Context& context, Session& session);

};

} // namespace anime::app
