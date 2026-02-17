cmake_minimum_required(VERSION 4.2.3)

set(PV_TESTING_NAME pvlib_test)

add_executable(
    ${PV_TESTING_NAME}
)

target_sources(
    ${PV_TESTING_NAME}
    PRIVATE
    PvPfTests.cpp
    PvPfCccTests.cpp
    PvStringUtilsTests.cpp
    PvInstrumentPanelDocumentTests.cpp
    PvInstrumentPanelDocumentDataNodeTests.cpp
    PvSettingsTests.cpp
    PvTestMain.cpp
    PvTestMain.hpp
)

set_target_properties(
    ${PV_TESTING_NAME}
    PROPERTIES
    WIN32_EXECUTABLE ON
)

target_link_libraries(${PV_TESTING_NAME}
    $ENV{DXSDK_DIR}/Lib/${PV_ARCH}/d3d9.lib
    $ENV{DXSDK_DIR}/Lib/${PV_ARCH}/d3dx9.lib
    BVETS_LIBRARY_OPTIONS
    pvlib
    gtest
)

# Register the testing suites to CTest
add_test(
    NAME
    MyTest
    COMMAND
    ${PV_TESTING_NAME}
)

target_include_directories(
    ${PV_TESTING_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src/Pv
)
