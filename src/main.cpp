//
//main.cpp
//
//Created by Luke Raykovitz on 4/17/25.

#include <iostream>

#include "io/fileHandling.hpp"
#include "databases/anime_data.hpp"



int main() {
	// Release / Build code 
	std::cout << "AnimeList.Backend. Build Date: " << __DATE__ << " Time: " << __TIME__ << "\n";

	animeDB anime;

	anime.loadAnimes("E:/Projects/Git/Anime-Database-Project/src/testFile.dat");


	return 0;
}