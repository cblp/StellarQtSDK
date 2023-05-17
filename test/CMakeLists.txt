set(CMAKE_AUTOMOC ON)

file(GLOB
    SOURCES
        "*.h"
        "federation/*.h"
        "requests/*.h"
        "responses/*.h"
        "main.cpp"
)
find_package(Qt6 REQUIRED COMPONENTS Core Network Test)
qt_add_executable(StellarQtSDKTest EXCLUDE_FROM_ALL ${SOURCES})
target_compile_definitions(StellarQtSDKTest PRIVATE
    STELLAR_ALLOW_UNSECURE_RANDOM
    STELLAR_ENABLE_TEST_METHODS
    STELLAR_SKIP_LIVE_TESTS
)
target_include_directories(StellarQtSDKTest PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(StellarQtSDKTest PRIVATE
    Qt::Network
    Qt::Test
    StellarQtSDK
)
add_test(NAME StellarQtSDKTest COMMAND StellarQtSDKTest)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS StellarQtSDKTest)
