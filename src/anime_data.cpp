//
//  Anime_data.cpp
//  Anime List
//
//  Created by Luke Raykovitz on 10/24/25.
//

#include "anime_data.hpp"



void animeDB::loadAnimes(std::string file) {
	parseAnimes(file);


	fileParsing parser;

	parser.parseFile(file, *this);
	
}

void animeDB::record(recordData& record, schemaData& schema) {
	int id = std::get<int>(record[0]);
	int expectedID = database.size();
	if (id != expectedID) {
		throw std::runtime_error("Non-sequential or corrupt ID in file.");
	}


	animeData anime;

	//for (int i = 0; i < record.size(); i++) {

	//	switch (schema[i].type) {

	//	case schemaField::dataType::U8: {

	//		break;
	//	}
	//	case schemaField::dataType::U16: {

	//		break;
	//	}
	//	case schemaField::dataType::U32: {

	//		break;
	//	}

	//	case schemaField::dataType::STR: {

	//		break;
	//	}

	//	case schemaField::dataType::VECU8: {
	//		
	//		break;
	//	}
	//	case schemaField::dataType::VECU16: {
	//		
	//		break;
	//	}

	//	default:
	//		throw std::runtime_error("Unknown Data Type in Schema! \n");
	//	}
	//}

	// Direct assignment Placeholder -- Will be replaced with dynamic assignment based on schema
	anime.rank = std::get<int>(record[1]);
	anime.title = std::get<std::string>(record[2]);
	anime.alt_title = std::get<std::string>(record[3]);
	anime.format = std::get<int>(record[4]);
	anime.status = std::get<int>(record[5]);
	anime.genres = std::get<std::vector<int>>(record[6]);
	anime.channels = std::get<std::vector<int>>(record[7]);
	anime.audios = std::get<std::vector<int>>(record[8]);
	anime.subtitles = std::get<std::vector<int>>(record[9]);
	anime.fan_service = std::get<int>(record[10]);
	anime.episode_count = std::get<int>(record[11]);
	anime.min_runtime = std::get<int>(record[12]);
	anime.characters = std::get<std::vector<int>>(record[13]);
	anime.ip_group = std::get<int>(record[14]);

	database.emplace_back(std::move(anime));
}

void animeDB::header(fileHeader& header) {
	if (header.type != 'A') {
		throw std::runtime_error("Wrong File Type!");
	}

	database.reserve(header.recordCount);
}

void animeDB::schema(schemaData& schema) {
	// should take the schema and use it to create a ordered vector of member pointers that can be used to assign data in the record function
}

std::vector<int> animeDB::getFilteredIDS(const Filters& f) {
	return {};
}

std::vector<int> animeDB::getSortedIDS() {
	return {};
}

void getData() {
	// placeholder for function that will print out the data in the database in a human readable format
}


const std::vector<std::string> service = {
	"Crunchyroll",
	"Netflix",
	"Hulu",
	"Disney Plus",
	"Amazon Prime Video",
	"HiDive",
	"Youtube"
};

const std::vector<std::string> format = {
	"Series",
	"Movie"
};

const std::vector<std::string> genre = {
	"Action",
	"Romance",
	"Slice of Life",
	"Comedy",
	"Drama",
	"Highschool",
	"College",
	"SciFi",
	"Modern Day",
	"Fantasy",
	"Ecchi",
	"Moving",
	"Magic",
	"Coming of Age",
	"Adventure",
	"Mystery",
	"Thriller",
	"Horror",
	"Supernatural",
	"Historical",
	"Mech",
	"Psychological",
	"Military",
	"Sports",
	"Music",
	"Martial Arts",
	"Shounen",
	"Shoujo",
	"Seinen",
	"Josei",
	"Isekai"
};

const std::vector<std::string> status = {
	"Ongoing",
	"Completed",
	"Limbo",
	"Cancelled",
	"Un-Released"
};

const std::vector<std::string> audio = {
	"Japanese",
	"English",
	"Korean",
	"Chinese",
	"Spanish",
	"French"
};

const std::vector<std::string> sub = {
	"Japanese",
	"English",
	"Korean",
	"Chinese",
	"Spanish",
	"French"
};

const std::vector<std::string> media = {
	"Manga",
	"Light Novel",
	"Movie",
	"Series",
	"Game"
};