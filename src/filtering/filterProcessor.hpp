#pragma once

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
#include <span>
#include <concepts>

#include "../io/fileHandling.hpp"

struct Range {
	std::optional<size_t> min;
	std::optional<size_t> max;
};

struct indexFilter {
	std::vector<size_t> includeAll;
	std::vector<size_t> includeAny;
	std::vector<size_t> exclude;
	Range range;
};

using indexFilters = std::vector<indexFilter>;

template<typename Database>
concept FilterableDatabase = requires(const Database& db, size_t index) {
	typename Database::IdType;
	{ db.getSize() } -> std::convertible_to<size_t>;
	{ db.getFieldCount() } -> std::convertible_to<size_t>;
	{ db.getFieldInfo(index).mode } -> std::convertible_to<schemaField::filterMode>;
	{ db.getField(index, index) } -> std::same_as<recordField>;
	{ db.getId(index) } -> std::same_as<typename Database::IdType>;
};


class filterProcessor {
	public:
		template<FilterableDatabase Database>
		auto checkFilters(const indexFilters& filters, const Database& database)
		-> std::vector<typename Database::IdType>;
	private:

		bool checkRange(const Range& r, size_t value);

		bool includesAny(const std::vector<size_t>& vec, size_t value);

		bool includesAny(const std::vector<size_t>& vec, const std::vector<size_t>& value);

		bool includesAll(const std::vector<size_t>& reference, const std::vector<size_t>& values);

		bool hasFilter(const indexFilter& filter);
};


#include "filterProcessor.tpp"


// 	Do you want to place a bet, 
// How much would you like to bet?
// var bet;
// What do you want to bet on?  even, odd, or input a number between 0 and 36?
// 
// 
// int rollRoulette()
// 
// 
// 
// 
