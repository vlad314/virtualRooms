cmake_minimum_required(VERSION 3.12)

project("virtualroom")

set(Boost_DEBUG ON)
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

if (WIN32)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHc /std:c++17")

	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT virtualroom)

else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++17")
endif()

find_package(Boost 1.68.0
	REQUIRED
	COMPONENTS program_options fiber regex date_time system)

add_definitions(-DBOOST_THREAD_PROVIDES_FUTURE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin")

add_subdirectory("src")