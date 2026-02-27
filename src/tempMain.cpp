#include <iostream>
#include "fileParsing.h"
#include "errors.h"
#include "anime_data.hpp"

int main() {
	animeDB anime;

	anime.loadAnimes("E:/Projects/Git/Anime-Database-Project/src/testFile.dat");

    return 0;
}
