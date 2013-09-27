set (CMAKE_BUILD_TYPE Debug)
FIND_LIBRARY(THREADLIB pthread)

#FIND_LIBRARY(LIBXML2 libxml2)
FIND_PACKAGE(LibXml2)

include_directories(${LIBXML2_INCLUDE_DIR})
add_executable (
	client
	client.cpp
)

#message("${CMAKE_SOURCE_DIR}")

TARGET_LINK_LIBRARIES(client ${THREADLIB} ${CMAKE_SOURCE_DIR}/../../libxmplib.so ${CMAKE_SOURCE_DIR}/../../libtinyxml.so)


