cmake_minimum_required(VERSION 3.20)
project(anime_database_project LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(unofficial-argon2 CONFIG REQUIRED)

find_package(CURL CONFIG REQUIRED)

if(WIN32)
	list(APPEND PROJECT_COMMON_LIBS Ws2_32 Bcrypt)
endif()

if(MSVC)
	add_compile_options(/EHsc)
endif()

if(WIN32)
    add_compile_definitions(NOMINMAX)
endif()

set(PROJECT_CORE_SOURCES
	"src/io/fileHandling.cpp"
	"src/app/entry_handler.cpp"
	"src/databases/library_service.cpp"
	"src/domain/entry_schema.cpp"
	"src/domain/group_schema.cpp"
	"src/domain/group_compiler.cpp"
	"src/databases/entry_database.cpp"
	"src/databases/user_database.cpp"
	"src/databases/group_database.cpp"
	"src/databases/auth/password.cpp"
	"src/databases/auth/auth_service.cpp"
	"src/filtering/filterProcessor.cpp"
	"src/protocol/entry_codec.cpp"
	"src/net/protocol/messenger.cpp"
	"src/net/protocol/data.cpp"
	"src/net/transport/net_service.cpp"
	"src/net/transport/session.cpp"
	"src/net/routing/router.cpp"
	"src/net/websocket/webSocket.cpp"
	"src/net/websocket/sha1.cpp"
	"src/net/websocket/base64.cpp"
	"src/net/https/httpCodec.cpp"
)

add_executable(anime_database_project
	"src/main.cpp"
	${PROJECT_CORE_SOURCES}
)

target_include_directories(anime_database_project PRIVATE include src)
target_link_libraries(anime_database_project PRIVATE ${PROJECT_COMMON_LIBS} unofficial::argon2::libargon2 CURL::libcurl)

add_custom_command(TARGET anime_database_project POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_RUNTIME_DLLS:anime_database_project>
		$<TARGET_FILE_DIR:anime_database_project>
	COMMAND_EXPAND_LISTS
)

add_executable(parser_stress_test
	"src/tests/parser_stress_test.cpp"
	"src/io/fileHandling.cpp")

 add_executable(net_debug
	"src/netDebug.cpp"
	${PROJECT_CORE_SOURCES}
 )
target_link_libraries(net_debug PRIVATE ${PROJECT_COMMON_LIBS} unofficial::argon2::libargon2 CURL::libcurl)
target_include_directories(net_debug PRIVATE src)


target_link_libraries(parser_stress_test PRIVATE ${PROJECT_COMMON_LIBS})
target_include_directories(parser_stress_test PRIVATE src)

add_test(NAME parser_stress_test COMMAND parser_stress_test)

add_executable(crunchyroll_import_test
	"src/tests/crunchyroll_import_test.cpp"
	"src/io/fileHandling.cpp"
	"src/domain/entry_schema.cpp"
	"src/domain/group_schema.cpp"
	"src/domain/group_compiler.cpp"
	"src/databases/entry_database.cpp"
	"src/databases/group_database.cpp"
	"src/filtering/filterProcessor.cpp")
target_include_directories(crunchyroll_import_test PRIVATE src)
target_link_libraries(crunchyroll_import_test PRIVATE ${PROJECT_COMMON_LIBS})
add_test(NAME crunchyroll_import_test COMMAND crunchyroll_import_test
	"${CMAKE_CURRENT_SOURCE_DIR}/src/io/animeDatabase.crunchyroll.dat"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/io/groupDatabase.crunchyroll.dat"
	"${CMAKE_CURRENT_BINARY_DIR}/crunchyroll_import_artifacts")

add_executable(parser_fuzz_test
	"src/tests/parser_fuzz_test.cpp"
	"src/io/fileHandling.cpp")

target_link_libraries(parser_fuzz_test PRIVATE ${PROJECT_COMMON_LIBS})
target_include_directories(parser_fuzz_test PRIVATE src)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/databases/auth/password.cpp")
	add_executable(auth_test
		"src/tests/auth_test.cpp"
		"src/io/fileHandling.cpp"
		"src/databases/user_database.cpp"
		"src/databases/auth/password.cpp"
		"src/databases/group_database.cpp"
		"src/domain/group_schema.cpp"
		"src/domain/group_compiler.cpp"
		"src/domain/entry_schema.cpp"
		"src/databases/entry_database.cpp"
		"src/filtering/filterProcessor.cpp"
	)

	target_link_libraries(auth_test PRIVATE ${PROJECT_COMMON_LIBS} unofficial::argon2::libargon2)
	target_include_directories(auth_test PRIVATE src)
	add_test(NAME auth_test COMMAND auth_test)
else()
	message(STATUS "Skipping auth_test: src/databases/auth/password.cpp is missing.")
endif()

add_test(NAME parser_fuzz_test COMMAND parser_fuzz_test)

add_executable(metadata_compile_test "src/tests/metadata_compile_test.cpp")
target_include_directories(metadata_compile_test PRIVATE src)
add_test(NAME metadata_compile_test COMMAND metadata_compile_test)

add_executable(entry_codec_test
	"src/tests/entry_codec_test.cpp"
	"src/protocol/entry_codec.cpp"
	"src/domain/entry_schema.cpp"
	"src/filtering/filterProcessor.cpp"
	"src/net/protocol/data.cpp"
)
target_include_directories(entry_codec_test PRIVATE src)
add_test(NAME entry_codec_test COMMAND entry_codec_test)

add_executable(entry_integration_test "src/tests/entry_integration_test.cpp" ${PROJECT_CORE_SOURCES})
target_include_directories(entry_integration_test PRIVATE src)
target_compile_definitions(entry_integration_test PRIVATE ANIME_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/src/io")
target_link_libraries(entry_integration_test PRIVATE ${PROJECT_COMMON_LIBS} unofficial::argon2::libargon2 CURL::libcurl)
add_test(NAME entry_integration_test COMMAND entry_integration_test)
add_test(NAME library_service_test COMMAND entry_integration_test --library-service)
