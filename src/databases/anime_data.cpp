//
//  Anime_data.cpp
//  Anime List
//
//  Created by Luke Raykovitz on 10/24/25.
//

#include "anime_data.hpp"



void animeDB::loadAnimes(std::string file) {
	fileHandler parser;

	parser.readFile(file, *this);
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
	// using chatgpt code to ensure that i only get correct values and to be type safe especialy against monostate
	anime.rank = std::get_if<int>(&record[1]) ? *std::get_if<int>(&record[1]) : 0;
	anime.title = std::get_if<std::string>(&record[2]) ? *std::get_if<std::string>(&record[2]) : "";
	anime.alt_title = std::get_if<std::string>(&record[3]) ? *std::get_if<std::string>(&record[3]) : "";
	anime.format = std::get_if<int>(&record[4]) ? *std::get_if<int>(&record[4]) : 0;
	anime.status = std::get_if<int>(&record[5]) ? *std::get_if<int>(&record[5]) : 0;
	anime.genres = std::get_if<std::vector<int>>(&record[6]) ? *std::get_if<std::vector<int>>(&record[6]) : std::vector<int>{};
	anime.channels = std::get_if<std::vector<int>>(&record[7]) ? *std::get_if<std::vector<int>>(&record[7]) : std::vector<int>{};
	anime.audios = std::get_if<std::vector<int>>(&record[8]) ? *std::get_if<std::vector<int>>(&record[8]) : std::vector<int>{};
	anime.subtitles = std::get_if<std::vector<int>>(&record[9]) ? *std::get_if<std::vector<int>>(&record[9]) : std::vector<int>{};
	anime.fan_service = std::get_if<int>(&record[10]) ? *std::get_if<int>(&record[10]) : 0;
	anime.episode_count = std::get_if<int>(&record[11]) ? *std::get_if<int>(&record[11]) : 0;
	anime.min_runtime = std::get_if<int>(&record[12]) ? *std::get_if<int>(&record[12]) : 0;
	anime.characters = std::get_if<std::vector<int>>(&record[13]) ? *std::get_if<std::vector<int>>(&record[13]) : std::vector<int>{};
	anime.ip_group = std::get_if<int>(&record[14]) ? *std::get_if<int>(&record[14]) : 0;

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

std::vector<size_t> animeDB::getFilteredIDS(const indexFilters& filters) {
	std::vector<size_t> result;

	filterProcessor processor;

	result = processor.checkFilters(filters, *this);

	return result;
}

std::vector<int> animeDB::getSortedIDS() {
	return {};
}


fileHeader animeDB::getHeader() {
	fileHeader result;
	result.type = 'A';
	result.recordCount = database.size();
	result.fieldCount = animeFieldMap.size();

	return result;
}


schemaData animeDB::getSchema() {
	schemaData result;

	for (size_t i = 0; i < animeFieldMap.size(); i++)
	{

	}

	return result;
}


recordData animeDB::getRecord() {
	recordData result;


	
	
	return result;
}

recordField animeDB::getField(size_t id, size_t fieldIndex)
{
	recordField result;

	const animeData& record = database[id];
	memPTR& ptr = orderedMemPTRS[fieldIndex];

	std::visit([&](auto memberPtr) {
			result = record.*memberPtr;
		}, ptr);

	return result;
}


std::vector<uint8_t> SerializeAnime::pack(const animeData& anime) {
	ByteWriter w;

	for (size_t i = 0; i < orderedMemPTRS.size(); i++) {





	}

	return w.buffer;
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