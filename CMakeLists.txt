cmake_minimum_required(VERSION 4.2.3)

set(BVETS_LIBRARY_NAME "PanelView")
set(BVETS_LIBRARY_THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/")

project(${BVETS_LIBRARY_NAME})

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PV_ARCH "x64")
else ()
    set(PV_ARCH "x86")
endif()


set_property(
  DIRECTORY PROPERTY
  VS_STARTUP_PROJECT
  ${BVETS_LIBRARY_NAME}
)

add_library(
    ${BVETS_LIBRARY_NAME}
    SHARED
)

add_library(
    BVETS_LIBRARY_OPTIONS
    INTERFACE
)

add_subdirectory(src)

target_compile_definitions(
    BVETS_LIBRARY_OPTIONS
    INTERFACE
    UNICODE
    _UNICODE
)

if("${PV_ARCH}" STREQUAL "x64")
    target_compile_definitions(
        BVETS_LIBRARY_OPTIONS
        INTERFACE
        _PV_ARCH_AMD64
    )
elseif("${PV_ARCH}" STREQUAL "x86")
    target_compile_definitions(
        BVETS_LIBRARY_OPTIONS
        INTERFACE
        _PV_ARCH_X86
    )
endif()

if(PV_DEBUG)
    target_compile_definitions(
        BVETS_LIBRARY_OPTIONS
        INTERFACE
        _PV_OPEN_DEBUG_CONSOLE
    )
endif()

if(PV_DEVELOPMENT)
    target_compile_definitions(
        BVETS_LIBRARY_OPTIONS
        INTERFACE
        _PV_DEVELOPMENT_BUILD
    )
endif()

target_compile_features(
    BVETS_LIBRARY_OPTIONS
    INTERFACE
    cxx_std_20
)

target_compile_options(
    BVETS_LIBRARY_OPTIONS
    INTERFACE
    "$<$<CONFIG:Debug>:/MDd>"
    "$<$<CONFIG:Release>:/MD>"
)

target_link_libraries(
    ${BVETS_LIBRARY_NAME}
    PRIVATE
    BVETS_LIBRARY_OPTIONS
    $ENV{DXSDK_DIR}/Lib/${PV_ARCH}/d3d9.lib
    $ENV{DXSDK_DIR}/Lib/${PV_ARCH}/d3dx9.lib
    pvlib
)

target_include_directories(
    ${BVETS_LIBRARY_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
    ${CMAKE_SOURCE_DIR}/Resharper.DotSettings
    ${CMAKE_BINARY_DIR}/${BVETS_LIBRARY_NAME}.sln.DotSettings
    COPYONLY
)

if (PV_TESTING)
    # Enable testing
    enable_testing()

    # Configure Google Test
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(${BVETS_LIBRARY_THIRDPARTY_DIR}/googletest)
    target_compile_features(
      gtest
      PRIVATE
      cxx_std_20
    )

    # Add testing suites
    add_subdirectory(testing)
endif()
