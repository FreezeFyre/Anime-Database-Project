#include "router.hpp"

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
