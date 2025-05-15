cmake_minimum_required(VERSION 3.10)

set(BVETS_LIBRARY_NAME "PanelView")
set(BVETS_SOURCE_FILE)

project(${BVETS_LIBRARY_NAME})

if(CMAKE_EXE_LINKER_FLAGS STREQUAL "/machine:x64")
    set(PV_ARCH "x64")
elseif(CMAKE_EXE_LINKER_FLAGS STREQUAL "/machine:X86")
    set(PV_ARCH "x86")
endif()


set_property(
  DIRECTORY PROPERTY
  VS_STARTUP_PROJECT
  ${BVETS_LIBRARY_NAME}
)

add_subdirectory(src)

set(BVETS_SOURCE_FILE
    src/atsplugin.h
    src/BveAtsPlugin.c
    src/BveAtsPlugin.h
    src/EntryPoint.cpp
)

add_library(
    ${BVETS_LIBRARY_NAME}
    SHARED
    ${BVETS_SOURCE_FILE}
)

target_compile_definitions(${BVETS_LIBRARY_NAME} PRIVATE UNICODE _UNICODE)

if("${PV_ARCH}" STREQUAL "x64")
    target_compile_definitions(${BVETS_LIBRARY_NAME} PRIVATE _PV_ARCH_AMD64)
elseif("${PV_ARCH}" STREQUAL "x86")
    target_compile_definitions(${BVETS_LIBRARY_NAME} PRIVATE _PV_ARCH_X86)
endif()

if(PV_DEBUG)
    target_compile_definitions(${BVETS_LIBRARY_NAME} PRIVATE _PV_OPEN_DEBUG_CONSOLE)
endif()

if(PV_DEVELOPMENT)
    target_compile_definitions(${BVETS_LIBRARY_NAME} PRIVATE _PV_DEVELOPMENT_BUILD)
endif()

set_target_properties(${BVETS_LIBRARY_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS ON
)

target_compile_options(${BVETS_LIBRARY_NAME} PRIVATE
"$<$<CONFIG:Debug>:/MDd>"
"$<$<CONFIG:Release>:/MD>"
)

target_link_libraries(
    ${BVETS_LIBRARY_NAME}
    PRIVATE
    $ENV{DXSDK_DIR}/Lib/${PV_ARCH}/d3d9.lib
    $ENV{DXSDK_DIR}/Lib/${PV_ARCH}/d3dx9.lib
    pvlib
)

target_include_directories(
    ${BVETS_LIBRARY_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (PV_TESTING)
    enable_testing()
    add_subdirectory(testing)
endif()