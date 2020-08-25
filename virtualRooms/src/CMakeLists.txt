cmake_minimum_required (VERSION 3.12)

add_executable(virtualroom
	virtualroomsrv_App.cpp
	virtualroomsrv_error.cpp
	virtualroomsrv_configreader.cpp
	virtualroomsrv_connectionutils.cpp
	virtualroomsrv_main.cpp
	virtualroomsrv_websocketconnection.cpp
	virtualroomsrv_httpconnection.cpp
	virtualroomsrv_listener.cpp	
)

target_include_directories(virtualroom PRIVATE ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(virtualroom PRIVATE ${Boost_LIBRARIES})

# TODO: this doesn't seem to work - investigate why
if (WIN32)
	set_property(GLOBAL PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()