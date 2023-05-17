find_package(Qt6 REQUIRED COMPONENTS Core Network)
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
target_include_directories(StellarQtSDK PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
)
target_link_libraries(StellarQtSDK PRIVATE
    cyoencode
    ed25519
    Qt::Core
    Qt::Network
    xdr
)
