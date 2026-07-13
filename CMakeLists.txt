cmake_minimum_required(VERSION 3.20)
project(anime_database_project LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(unofficial-argon2 CONFIG REQUIRED)

if(WIN32)
	list(APPEND PROJECT_COMMON_LIBS Ws2_32 Bcrypt)
endif()

set(PROJECT_CORE_SOURCES
	"src/io/fileHandling.cpp"
	"src/databases/entry_data.cpp"
	"src/filtering/filterProcessor.cpp"
	"src/net/protocol/messenger.cpp"
	"src/net/protocol/data.cpp"
	"src/net/transport/net_service.cpp"
	"src/net/routing/router.cpp"
	"src/net/websocket/webSocket.cpp"
	"src/net/websocket/sha1.cpp"
	"src/net/websocket/base64.cpp"
	"src/databases/users.cpp"
	"src/databases/groups.cpp"
	"src/databases/auth/password.cpp"
	"src/databases/auth/auth_service.cpp"
)

set(PROJECT_ENTRY_TEST_SOURCES
	"src/io/fileHandling.cpp"
	"src/databases/entry_data.cpp"
	"src/filtering/filterProcessor.cpp"
	"src/net/protocol/messenger.cpp"
	"src/net/protocol/data.cpp"
	"src/databases/users.cpp"
	"src/databases/groups.cpp"
)

add_executable(anime_database_project
	"src/main.cpp"
	${PROJECT_CORE_SOURCES}
)

target_include_directories(anime_database_project PRIVATE include)
target_link_libraries(anime_database_project PRIVATE ${PROJECT_COMMON_LIBS} unofficial::argon2::libargon2)

add_executable(parser_stress_test
	"src/tests/parser_stress_test.cpp"
	"src/io/fileHandling.cpp"
)

target_link_libraries(parser_stress_test PRIVATE ${PROJECT_COMMON_LIBS})

add_test(NAME parser_stress_test COMMAND parser_stress_test)

add_executable(parser_fuzz_test
	"src/tests/parser_fuzz_test.cpp"
	"src/io/fileHandling.cpp"
)

target_link_libraries(parser_fuzz_test PRIVATE ${PROJECT_COMMON_LIBS})

add_executable(entry_update_test
	"src/tests/entry_update_test.cpp"
	${PROJECT_ENTRY_TEST_SOURCES}
)

target_link_libraries(entry_update_test PRIVATE ${PROJECT_COMMON_LIBS})

add_executable(entry_search_test
	"src/tests/entry_search_test.cpp"
	${PROJECT_ENTRY_TEST_SOURCES}
)

target_link_libraries(entry_search_test PRIVATE ${PROJECT_COMMON_LIBS})

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/users/auth.cpp" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/users/auth.hpp")
	add_executable(auth_test
		"src/tests/auth_test.cpp"
		"src/io/fileHandling.cpp"
		"src/databases/users.cpp"
		"src/users/auth.cpp"
		"src/databases/groups.cpp"
	)

	target_link_libraries(auth_test PRIVATE ${PROJECT_COMMON_LIBS})
	add_test(NAME auth_test COMMAND auth_test)
else()
	message(STATUS "Skipping auth_test: src/users/auth.cpp or src/users/auth.hpp is missing.")
endif()

add_test(NAME parser_fuzz_test COMMAND parser_fuzz_test)
add_test(NAME entry_update_test COMMAND entry_update_test)
add_test(NAME entry_search_test COMMAND entry_search_test)
