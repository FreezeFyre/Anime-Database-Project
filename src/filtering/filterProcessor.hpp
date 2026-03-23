#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <limits>
#include <variant>
#include <unordered_map>
#include <array>
#include <optional>

#include "../io/fileHandling.hpp"

struct range {
	std::optional<size_t> min;
	std::optional<size_t> max;
};

struct indexFilter {
	std::vector<size_t> includeAll;
	std::vector<size_t> includeAny;
	std::vector<size_t> exclude;
	range range;
};

using indexFilters = std::vector<indexFilter>;

class filterProcessor {
	public:
		template<typename f>
		std::vector<size_t> checkFilters(const indexFilters& filters, f& database);
	private:

		bool checkRange(const range& r, size_t value);

		bool includesAny(const std::vector<size_t>& vec, size_t value);

		bool includesAny(const std::vector<size_t>& vec, const std::vector<size_t>& value);

		bool  includesAll(const std::vector<size_t>& reference, const std::vector<size_t>& values);

		bool hasFilter(const indexFilter& filter);
};


#include "filterProcessor.tpp"