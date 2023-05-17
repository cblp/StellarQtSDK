find_package(Qt6 REQUIRED COMPONENTS Core Network)
include_directories("${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_AUTOMOC ON)

file(
    GLOB
    SOURCES
        "./*"
        "./federation/*"
        "./requests/*"
        "./responses/*"
        "./responses/effects/*"
        "./responses/operations/*"
)
qt_add_library(StellarQtSDK ${SOURCES})
target_link_libraries(StellarQtSDK PUBLIC
    cyoencode
    ed25519
    Qt::Core
    Qt::Network
    xdr
)
