cmake_minimum_required(VERSION 3.20)
project(anime_database_project LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_library(argon2 STATIC
	"third_party/argon2/src/argon2.c"
	"third_party/argon2/src/core.c"
	"third_party/argon2/src/encoding.c"
	"third_party/argon2/src/ref.c"
	"third_party/argon2/src/blake2/blake2b.c")

target_compile_definitions(argon2 PRIVATE ARGON2_NO_THREADS)
target_include_directories(argon2
	PUBLIC "third_party/argon2/include"
	PRIVATE "third_party/argon2/src"
)

set(PROJECT_COMMON_LIBS argon2)

if(WIN32)
	list(APPEND PROJECT_COMMON_LIBS Ws2_32 Bcrypt)
endif()

add_executable(anime_database_project

	"src/main.cpp"
	"src/io/fileHandling.cpp"
	"src/databases/entry_data.cpp"
	"src/filtering/filterProcessor.cpp"
	
	"src/net/protocol/messenger.cpp"
	"src/net/protocol/data.cpp"

 "src/net/transport/net_service.cpp" "src/app.cpp" "src/net/routing/router.cpp" "src/net/websocket/webSocket.cpp" "src/net/websocket/sha1.cpp" "src/net/websocket/base64.cpp" "src/databases/users.cpp" "src/users/auth.cpp" "src/databases/groups.cpp")


target_include_directories(anime_database_project PRIVATE include)
target_link_libraries(anime_database_project PRIVATE ${PROJECT_COMMON_LIBS})

add_executable(parser_stress_test
	"src/tests/parser_stress_test.cpp"
	"src/io/fileHandling.cpp" "src/net/websocket/webSocket.cpp" "src/net/websocket/sha1.cpp" "src/net/websocket/base64.cpp" "src/databases/users.cpp" "src/users/auth.cpp" "src/databases/groups.cpp")

target_link_libraries(parser_stress_test PRIVATE ${PROJECT_COMMON_LIBS})

add_test(NAME parser_stress_test COMMAND parser_stress_test)

add_executable(parser_fuzz_test
	"src/tests/parser_fuzz_test.cpp"
	"src/io/fileHandling.cpp" "src/net/websocket/webSocket.cpp" "src/net/websocket/sha1.cpp" "src/net/websocket/base64.cpp" "src/databases/users.cpp" "src/users/auth.cpp" "src/databases/groups.cpp")

target_link_libraries(parser_fuzz_test PRIVATE ${PROJECT_COMMON_LIBS})

add_executable(entry_update_test
	"src/tests/entry_update_test.cpp"
	"src/io/fileHandling.cpp"
	"src/databases/entry_data.cpp"
	"src/filtering/filterProcessor.cpp"
	"src/net/protocol/messenger.cpp"
	"src/net/protocol/data.cpp" "src/databases/users.cpp" "src/users/auth.cpp" "src/databases/groups.cpp")

target_link_libraries(entry_update_test PRIVATE ${PROJECT_COMMON_LIBS})

add_executable(entry_search_test
	"src/tests/entry_search_test.cpp"
	"src/io/fileHandling.cpp"
	"src/databases/entry_data.cpp"
	"src/filtering/filterProcessor.cpp"
	"src/net/protocol/messenger.cpp"
	"src/net/protocol/data.cpp" "src/databases/users.cpp" "src/users/auth.cpp" "src/databases/groups.cpp")

target_link_libraries(entry_search_test PRIVATE ${PROJECT_COMMON_LIBS})

add_executable(auth_test
	"src/tests/auth_test.cpp"
	"src/io/fileHandling.cpp"
	"src/databases/users.cpp"
	"src/users/auth.cpp"
 "src/databases/groups.cpp")

target_link_libraries(auth_test PRIVATE ${PROJECT_COMMON_LIBS})

add_test(NAME parser_fuzz_test COMMAND parser_fuzz_test)
add_test(NAME entry_update_test COMMAND entry_update_test)
add_test(NAME entry_search_test COMMAND entry_search_test)
add_test(NAME auth_test COMMAND auth_test)
