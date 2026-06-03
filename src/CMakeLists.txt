cmake_minimum_required(VERSION 4.2.3)

target_sources(
    ${BVETS_LIBRARY_NAME}
    PRIVATE
    atsplugin.h
    BveAtsPlugin.c
    BveAtsPlugin.h
    EntryPoint.cpp
)

add_subdirectory(pv)
