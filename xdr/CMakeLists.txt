file(GLOB SOURCES "./*")

include_directories("${CMAKE_SOURCE_DIR}")

find_package(Qt6 COMPONENTS Core REQUIRED)

qt_add_library(xdr STATIC)
target_sources(xdr PUBLIC ${SOURCES})
target_link_libraries(xdr PUBLIC Qt6::Core)
