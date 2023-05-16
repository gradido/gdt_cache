cmake_minimum_required(VERSION 3.0)
project(GdtCache CXX)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin" )

IF(WIN32)
	set(CMAKE_CXX_FLAGS         "/MP /EHsc /std:c++17")
ELSE()
	SET(CMAKE_CXX_FLAGS "-std=c++17" )
ENDIF()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(INSTALL_BINDIR "bin")
set(INSTALL_PLUGINDIR "bin")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

FILE(GLOB API "src/api/*.cpp" "src/api/*.h")
FILE(GLOB MAIN "src/*.cpp" "src/*.h")
FILE(GLOB VIEW "src/view/*.cpp" "src/view/*.h")
FILE(GLOB CONTROLLER "src/controller/*.cpp" "src/controller/*.h")
FILE(GLOB MYSQL "src/mysql/*.cpp" "src/mysql/*.h")
FILE(GLOB MODEL "src/model/*.cpp" "src/model/*.h")
FILE(GLOB UTILS "src/utils/*.cpp" "src/utils/*.h")
FILE(GLOB BACKGROUND_TASKS "src/background_tasks/*.cpp" "src/background_tasks/*.h")
FILE(GLOB LOG "src/logging/*.cpp" "src/logging/*.h")

SET(LOCAL_SRCS ${MAIN} ${CONTROLLER} ${MODEL} ${VIEW} ${API} ${UTILS} ${TASK} ${BACKGROUND_TASKS} ${LOG} ${MYSQL})

add_subdirectory(third_party/mariadb-connector-c)

find_package(CURL REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED context)

include_directories(${CURL_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIR})

set(LIBS  ${CURL_LIBRARIES} libmariadb
          ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
          ${OPENSSL_LIBRARIES})

# Directory where lithium headers are located if not in the default include path.
include_directories(
    third_party/lithium/single_headers
    third_party/rapidjson/include
    third_party/mariadb-connector-c/include
    ${CMAKE_BINARY_DIR}/third_party/mariadb-connector-c/include    
)

#add_custom_target(symbols_generation COMMAND li_symbol_generator ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(GdtCache ${LOCAL_SRCS})
#add_dependencies(GdtCache symbols_generation)
target_link_libraries(GdtCache ${LIBS})