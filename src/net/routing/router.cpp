#include "router.hpp"

AppResponse Router::route(AppRequest& request, Context& context, Session& session) {
	switch (request.domain) {
		case DOMAIN_TYPE::ENTRY: {
			EntryRouter eRoute;
			const EntryRequest& req = std::get<EntryRequest>(request.payload);
			EntryResponse resp = eRoute.route(req, context, session);
			return {request.id, resp, resp.closeConnection};
		}
		
		
		default:
			throw std::runtime_error("[ERROR][App Router] \t Unexpected/Invalid Domain Type");
	}
}
