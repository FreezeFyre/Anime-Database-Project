cmake_minimum_required(VERSION 3.20)
project(anime_database_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_executable(anime_database_project

	"src/main.cpp"
	"src/io/fileHandling.cpp"
	"src/databases/anime_data.cpp"
	"src/filtering/filterProcessor.cpp"
	"src/net/tcp.cpp"
	"src/net/http.cpp"
	"src/net/data.cpp"

 "src/net/net_stack.cpp")


target_include_directories(anime_database_project PRIVATE include)

add_executable(parser_stress_test
	"src/tests/parser_stress_test.cpp"
	"src/io/fileHandling.cpp"
 "src/net/net_stack.cpp")

add_test(NAME parser_stress_test COMMAND parser_stress_test)

add_executable(parser_fuzz_test
	"src/tests/parser_fuzz_test.cpp"
	"src/io/fileHandling.cpp"
 "src/net/net_stack.cpp")

add_test(NAME parser_fuzz_test COMMAND parser_fuzz_test)
