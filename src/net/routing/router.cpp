#include "net/routing/router.hpp"

namespace anime::app {

AppResponse Router::route(AppRequest& request, Context& context, Session& session) {
	return std::visit([&](const auto& payload) {
		return handle(payload, request.id, context, session);
	},request.payload);
}

AppResponse Router::handle(const EntryRequest & request, uint32_t id, Context & context, Session & session) {
	EntryRouter router;
	EntryResponse resp = router.route(request, context, session);
	return AppResponse{id, resp, resp.closeConnection};
}

AppResponse Router::handle(const AuthRequest& request, uint32_t id, Context& context, Session& session) {
	AuthRouter router;
	AuthResponse resp = router.route(request, context, session);
	return AppResponse{id, resp, false}; // Bool is false as it is ignored for http
}

} // namespace anime::app
