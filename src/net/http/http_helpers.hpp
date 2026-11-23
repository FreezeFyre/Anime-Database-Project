#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace anime::net::http {
using Headers = std::vector<std::pair<std::string,std::string>>;

Headers parseHeaders(std::string_view headers);
std::vector<std::string> getHeaderValues(const Headers& headers, std::string_view header);
std::string retrieveCookie(const Headers& headers, std::string_view target);

} // namespace anime::net::http
