
FIND_LIBRARY(THREADLIB pthread)

add_executable (
	request
	RequestRound.cpp
)

#message("${CMAKE_SOURCE_DIR}")

TARGET_LINK_LIBRARIES(request ${THREADLIB} ${CMAKE_SOURCE_DIR}/../../libxmplib.so ${CMAKE_SOURCE_DIR}/../../libtinyxml.so)


