cmake_minimum_required(VERSION 3.18)

project(StellarQtSDK)

add_definitions(
    -DSTELLAR_QT_REPLY_TIMEOUT=30000

    -DED25519_NO_SEED

    -DSTELLAR_QT_SDK_CLIENT_NAME="qtcpp-stellar-sdk"
    -DSTELLAR_QT_SDK_CLIENT_VERSION="0.3.17"

    # -DSTELLAR_QT_AUTOSET_BASE_FEE

    # If you have problems with network connections, uncomment this define to print some traces that may help you.
    # -DSTELLAR_QT_DEBUG_NETWORK_REQUESTS
)

add_subdirectory(external/cyoencode)
add_subdirectory(external/ed25519)

add_subdirectory(xdr)
add_subdirectory(src)

add_subdirectory(test)
