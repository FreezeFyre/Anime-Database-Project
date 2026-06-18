#include "filterProcessor.hpp"

bool filterProcessor::checkRange(const Range& r, size_t value) {
	if (r.max and value > *r.max) {
		return false;
	}
	if (r.min and value < *r.min) {
		return false;
	}
	return true;
}




bool filterProcessor::includesAny(const std::vector<size_t>& vec, size_t value) {
	/*


	I can probably replace the loop and all its logic with a std::find


	*/
	for (size_t i = 0; i < vec.size(); i++) {
		if (vec[i] == value) {
			return true;
		}
	}
	return false;
}


bool filterProcessor::includesAny(const std::vector<size_t>& vec, const std::vector<size_t>& value) {
	/*
	
	
	May want to replace with a unordered_set and use find to scale for large vectors
	and avoid O(n^2) time and get O(n) build and O(1) lookup
	
	
	*/
	for (size_t i = 0; i < value.size(); i++) {
		for (size_t n = 0; n < vec.size(); n++) {
			if (value[i] == vec[n])
			{
				return true;
			}
		}
	}
	return false;
}



bool filterProcessor::includesAll(const std::vector<size_t>& reference, const std::vector<size_t>& values) {
	/*
	
	
	May want to replace with a unordered_set and use find to scale for large vectors
	and avoid O(n^2) time and get O(n) build and O(1) lookup
	
	
	*/
	for (size_t i = 0; i < values.size(); i++) {
		if (!includesAny(reference, values[i])) {
			return false;
		}
	}
	return true;
}




bool filterProcessor::hasFilter(const indexFilter& filter) {
	return
		!filter.includeAll.empty() ||
		!filter.includeAny.empty() ||
		!filter.exclude.empty() ||
		filter.range.min.has_value() ||
		filter.range.max.has_value();
		
}
