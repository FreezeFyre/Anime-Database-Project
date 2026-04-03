cmake_minimum_required(VERSION 3.20)
project(anime_database_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(anime_database_project

	"src/main.cpp"
	"src/io/fileHandling.cpp"
	"src/databases/anime_data.cpp"
	"src/filtering/filterProcessor.cpp"
	"src/net/tcp.cpp"
	"src/net/http.cpp"
	"src/net/data.cpp"

)


target_include_directories(anime_database_project PRIVATE include)
