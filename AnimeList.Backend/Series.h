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

#pragma once

struct animeGroup {
	int rank;
	std::string title;
	std::string alt_title;
	int status;
	std::vector<int> genres;   // Genres that ever entre has//
	std::vector<int> channels; // Channels that ever entre has//
	std::vector<int> audios;    // Audio languages that ever entre has//
	std::vector<int> subtitles; // Subtitle languages that ever entre has//
	int fan_service_rating; // Average or Highest fan service rating among entries//
	int season_count;
	int film_count; 
	int episode_count;
	int avg_episode_length;
	int min_runtime;
	std::vector<int> characters; // All Characters that are present//
	std::vector<int> anime_ids;
	std::vector<int> related_media;
};

extern std::unordered_map<int, animeGroup> ipGroup;

class groupDatabase {
	public:

	private:

};