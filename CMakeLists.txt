


cmake_minimum_required(VERSION 2.8)

set (CMAKE_BUILD_TYPE Debug)
add_subdirectory(tinyxml++)

add_library(
	tinyxml
	SHARED
	tinyxml++/ticpp.cpp  
 	tinyxml++/tinystr.cpp  
 	tinyxml++/tinyxml.cpp
 	tinyxml++/tinyxmlerror.cpp  
 	tinyxml++/tinyxmlparser.cpp
	
)


FIND_LIBRARY(THREADLIB pthread)


####Das sollte optional sein:
FIND_LIBRARY(LIBXML2 libxml2)
FIND_PACKAGE(LibXml2)

include_directories(${LIBXML2_INCLUDE_DIR})
############################

set (
   SOURCES_OLD
	ClientSocket.cpp 
	Socket.cpp
	src/Log.cpp
)

add_executable (
	xmpd
	Manager.cpp
	${SOURCES_OLD}
)
#add_executable (
#	test
#	tinyxmltest.cpp
#	${SOURCES_OLD}
#)

add_library(
	xmplib
	SHARED
	xmplib.cpp
	${SOURCES_OLD}
)
#message("${CMAKE_SOURCE_DIR}")

#TARGET_LINK_LIBRARIES(manager ${THREADLIB} ${CMAKE_SOURCE_DIR}/tinyxml++/libtinyxml.so)
#TARGET_LINK_LIBRARIES(test ${CMAKE_SOURCE_DIR}/tinyxml++/libtinyxml.so)

TARGET_LINK_LIBRARIES(xmpd ${THREADLIB} ${CMAKE_SOURCE_DIR}/libtinyxml.so)
#TARGET_LINK_LIBRARIES(test ${CMAKE_SOURCE_DIR}/tinyxml++/libtinyxml.so)


TARGET_LINK_LIBRARIES(xmplib ${LIBXML2_LIBRARIES} )
