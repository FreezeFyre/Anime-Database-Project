//
//main.cpp
//
//Created by Luke Raykovitz on 4/17/25.

#include <iostream>

#include "io/fileHandling.hpp"
#include "databases/anime_data.hpp"
#include "filtering/filterProcessor.hpp"
#include "net/data.hpp"



int main() {
	// Release / Build code 
	std::cout << "AnimeList.Backend. Build Date: " << __DATE__ << " Time: " << __TIME__ << "\n";

	animeDB anime;

	anime.loadAnimes("E:/Projects/Git/Anime-Database-Project/src/testFile.dat");

	ByteWriter w;

	uint32_t v = 12578;

	w.U32(v);

	for (size_t i = 0; i < w.buffer.size(); i++)
	{
		std::cout << w.buffer[i] << std::endl;
	}

	return 0;
}