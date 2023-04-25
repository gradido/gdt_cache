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

FILE(GLOB API "src/api/*.cpp" "src/api/*.h")
FILE(GLOB MAIN "src/*.cpp" "src/*.h")
FILE(GLOB CONTROLLER "src/controller/*.cpp" "src/controller/*.h")
FILE(GLOB MODEL "src/model/*.cpp" "src/model/*.h")
FILE(GLOB LIB "src/lib/*.cpp" "src/lib/*.h")
FILE(GLOB TASK "src/task/*.cpp" "src/task/*.h")

SET(LOCAL_SRCS ${MAIN} ${CONTROLLER} ${MODEL} ${API} ${LIB} ${TASK})

if(MSVC)
    source_group("api" FILES ${API})
    source_group("controller" FILES ${CONTROLLER})
    source_group("lib" FILES ${LIB})
    source_group("model" FILES ${MODEL})    
    source_group("task" FILES ${TASK})
endif(MSVC)

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