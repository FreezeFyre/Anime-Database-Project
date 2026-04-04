//
//  Anime_data.cpp
//  Anime List
//
//  Created by Luke Raykovitz on 10/24/25.
//
#include "anime_data.hpp"



void animeDB::loadAnimes(std::string file) {
	fileProcessor parser;

	try {
		parser.readFile(file, *this, fileType::HEADER_DATABASE);
	}
	catch (const std::exception& e) {
		std::cerr << "Error Loading Anime Database: " << e.what() << "\n";
	}
}

void animeDB::loadLookupTables(std::string file) {
	fileProcessor parser;

	try {
		parser.readFile(file, *this, fileType::HEADER_TABLE);
	}
	catch (const std::exception& e) {
		std::cerr << "Error Loading Anime Lookup Tables: " << e.what() << "\n";
	}
}

void animeDB::table(std::string& table, std::string& title) {
	if (title == "ANIME_TAG_GENRES") {
		lookup.genres.push_back(table);
	}
	else if (title == "ANIME_TAG_CHANNELS") {
		lookup.channels.push_back(table);
	}
	else if (title == "ANIME_TAG_AUDIOS") {
		lookup.audios.push_back(table);
	}
	else if (title == "ANIME_TAG_SUBTITLES") {
		lookup.subtitles.push_back(table);
	}
	else if (title == "ANIME_TAG_FORMATS") {
		lookup.formats.push_back(table);
	}
	else if (title == "ANIME_TAG_STATUSES") {
		lookup.statuses.push_back(table);
	}
	else if (title == "ANIME_TAG_MEDIAS") {
		lookup.medias.push_back(table);
	}
	else {
		throw std::runtime_error("Unknown Table Title: " + title);
	}
}

// Still Need to Add Dynamic Assignment Based on Schema, Currently Just a Placeholder for Testing
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

// Reserves the space for all records in th at database
void animeDB::section(sectionHeader& header) {

	database.reserve(header.recordCount);
}

void animeDB::schema(schemaData& schema) {
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

	return result;
}

sectionHeader animeDB::getSection() {
	sectionHeader result;

	result.type = sectionType::SECTION_DATABASE_RECORDS; // Placeholder
	result.title = "ANIME_DATA";
	result.recordCount = database.size();
	result.fieldCount = animeMeta.size();
	result.hasSchema = true;

	return result;
}

schemaData animeDB::getSchema() {
	schemaData result;

	for (size_t i = 0; i < animeMeta.size(); i++)
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
	const memPTR& ptr = animeMeta[fieldIndex].ptr;

	std::visit([&](auto memberPtr) {
			result = record.*memberPtr;
		}, ptr);

	return result;
}



std::vector<uint8_t> animeSerializer::pack(const animeData& anime) {
	ByteWriter w;

	for (size_t i = 0; i < animeMeta.size(); i++) {
		switch (animeMeta[i].type) {
			case schemaField::dataType::U8: {
				w.U8(anime.*std::get<0>(animeMeta[i].ptr));
				break;
			}
			case schemaField::dataType::U16: {
				w.U16(anime.*std::get<0>(animeMeta[i].ptr));
				break;
			}
			case schemaField::dataType::U32: {
				w.U32(anime.*std::get<0>(animeMeta[i].ptr));
				break;
			}
			case schemaField::dataType::STR: {
				w.STR(anime.*std::get<2>(animeMeta[i].ptr));
				break;
			}
			case schemaField::dataType::VECU8: {
				w.VECU8(anime.*std::get<1>(animeMeta[i].ptr));
				break;
			}
			case schemaField::dataType::VECU16: {
				w.VECU16(anime.*std::get<1>(animeMeta[i].ptr));
				break;
			}
			default:
				throw std::runtime_error("Unknown Data Type in animeMeta field index: " + std::to_string(i));
		}
	}

	return w.buffer;
}

animeData animeSerializer::unpack(const std::vector<uint8_t>& bytes) {
	animeData entree;

	std::vector<uint8_t> buffer;
	for (size_t i = 0; i < bytes.size(); i++) {

	}

	return entree;
}
