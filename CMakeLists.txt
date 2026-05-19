cmake_minimum_required(VERSION 3.20)
project(anime_database_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_executable(anime_database_project

	"src/main.cpp"
	"src/io/fileHandling.cpp"
	"src/databases/entry_data.cpp"
	"src/filtering/filterProcessor.cpp"
	
	"src/net/protocol/messenger.cpp"
	"src/net/protocol/data.cpp"

 "src/net/transport/net_service.cpp" "src/app.cpp" "src/net/routing/router.cpp" "src/net/websocket/webSocket.cpp" "src/net/websocket/sha1.cpp" "src/net/websocket/base64.cpp")


target_include_directories(anime_database_project PRIVATE include)
target_compile_definitions(anime_database_project PRIVATE PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

if(WIN32)
	target_link_libraries(anime_database_project PRIVATE Ws2_32)
endif()

add_executable(parser_stress_test
	"src/tests/parser_stress_test.cpp"
	"src/io/fileHandling.cpp" "src/net/websocket/webSocket.cpp" "src/net/websocket/sha1.cpp" "src/net/websocket/base64.cpp")

add_test(NAME parser_stress_test COMMAND parser_stress_test)

add_executable(parser_fuzz_test
	"src/tests/parser_fuzz_test.cpp"
	"src/io/fileHandling.cpp" "src/net/websocket/webSocket.cpp" "src/net/websocket/sha1.cpp" "src/net/websocket/base64.cpp")

if(WIN32)
	target_link_libraries(parser_stress_test PRIVATE Ws2_32)
	target_link_libraries(parser_fuzz_test PRIVATE Ws2_32)
endif()

add_test(NAME parser_fuzz_test COMMAND parser_fuzz_test)
