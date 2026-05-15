#include "router.hpp"

RouteResult Router::route(Message& message, Context& context) {
	RouteResult result;
	ByteReader reader{ message.payload };
	// This is where the routing logic will go, for now it just prints the message type
	if (message.comType == COM_TYPE::REQ) {
		switch (message.domain) {
			case DOMAIN_TYPE::ENTRY:
				// Handle entry domain actions
				switch (message.action) {
					case ACTION_TYPE::ACT_GET:
						// Handle get action for entry domain
						// Convert payload to get parameters, get parameters in this case should probably be a vector of ids to get or single id 0 for all E.X. [1, 2, 3] or [0]



						break;
					case ACTION_TYPE::ACT_UPDATE:
						// Handle update action for entry domain


						break;
					case ACTION_TYPE::ACT_DELETE:
						// Handle delete action for entry domain


						break;
					case ACTION_TYPE::ACT_SEARCH:
						// Handle search action for entry domain


						break;
					default:
						std::cerr << "Unknown action type: " << static_cast<int>(message.action) << "\n";
						break;
				}
				break;
			case DOMAIN_TYPE::CHARACTERS:
				// Handle characters domain actions
				break;
			case DOMAIN_TYPE::FILTERS:
				// Handle filters domain actions
				break;
			case DOMAIN_TYPE::AUTH:
				// Handle auth domain actions
				break;
			default:
				std::cerr << "Unknown domain type: " << static_cast<int>(message.domain) << "\n";
				break;
		}

	}
	else if (message.comType == COM_TYPE::RESP) {
	
	
	}
	else if (message.comType == COM_TYPE::STATUS) {
	
	}
	else if (message.comType == COM_TYPE::PING) {
		result.message.encodeVersion = ENCODE_VERSION;
		result.message.comType = COM_TYPE::RESP;
		result.message.domain = message.domain;
		result.message.action = message.action;
		result.message.id = message.id;
		result.message.payloadSize = 4;
		result.message.payload = std::vector<uint8_t>({'P', 'O', 'N', 'G'});

		result.closeConnection = true;
		return result;
	}
	else {
		std::cerr << "Unknown COM_TYPE: " << static_cast<int>(message.comType) << "\n";

		std::string errorMsg = "Unknown COM_TYPE: " + std::to_string(static_cast<int>(message.comType));

		result.message.encodeVersion = ENCODE_VERSION;
		result.message.comType = COM_TYPE::RESP;
		result.message.domain = message.domain;
		result.message.action = message.action;
		result.message.id = message.id;
		result.message.payloadSize = static_cast<uint32_t>(errorMsg.size());
		result.message.payload = std::vector<uint8_t>(errorMsg.begin(), errorMsg.end());

		result.closeConnection = true;
		return result;
	}

	return result;

}
