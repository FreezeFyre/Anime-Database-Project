cmake_minimum_required(VERSION 3.20)
project(anime_database_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(anime_database_project
	"src/tempMain.cpp"
	"src/tempmain.cpp"
)


target_include_directories(anime_database_project PRIVATE include)
