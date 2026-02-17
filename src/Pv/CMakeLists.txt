cmake_minimum_required(VERSION 4.2.3)

set(PV_LIB_NAME pvlib)
add_library(${PV_LIB_NAME} STATIC)

target_sources(${PV_LIB_NAME}
    PRIVATE
    PvBridgingDefinitions.h
    PvBridgingUtils.hpp
    PvMain.cpp
    PvMain.h
    PvSettingsTomlUtils.hpp
    PvSettingsTomlUtils.cpp
    PvSettingsBase.hpp
    PvSettings_Root.hpp
    PvSettings_Root.cpp
    PvSettings_Window.hpp
    PvSettings_Window.cpp
    PvObject.hpp
    PvAtomicFlag.hpp
    PvStringUtils.hpp
    PvStringUtils.cpp
    PvPf_Windows.cpp
    PvPf_Windows.hpp
    PvLogHandler.hpp
    PvLogHandler.cpp
    PvLogHandlerParser.hpp
    PvLogHandlerParser.cpp
    PvWindowBase.cpp
    PvWindowBase.hpp
    PvPanelWindow.cpp
    PvPanelWindow.hpp
    PvGr_D3D9.cpp
    PvGr_D3D9.hpp
    PvEnvironmentInformationPublisher.hpp
    PvEnvironmentInformationPublisher.cpp
    PvEnvironmentInformationSubscriberBase.hpp
    PvEnvironmentInformationSubscriberBase.cpp
    PvEnvironmentInformationSubject.hpp
    PvEnvironmentInformationSubject.cpp
    PvInstrumentPanelDefinitions.hpp
    PvInstrumentPanelDocumentDataNode.hpp
    PvInstrumentPanelDocument.cpp
    PvInstrumentPanelDocument.hpp
    PvInstrumentPanelComponent_CircularMeterObjectBase.hpp
    PvInstrumentPanelComponent_ParametricObjectBase.hpp
    PvInstrumentPanelComponent_TextureMappingObjectBase.hpp
    PvInstrumentPanelComponentBase.cpp
    PvInstrumentPanelComponentBase.hpp
    PvInstrumentPanelComponent_This.cpp
    PvInstrumentPanelComponent_This.hpp
    PvInstrumentPanelComponent_PilotLamp.cpp
    PvInstrumentPanelComponent_PilotLamp.hpp
    PvInstrumentPanelComponent_Needle.cpp
    PvInstrumentPanelComponent_Needle.hpp
    PvInstrumentPanelComponent_DigitalNumber.cpp
    PvInstrumentPanelComponent_DigitalNumber.hpp
    PvInstrumentPanelComponent_DigitalGauge.cpp
    PvInstrumentPanelComponent_DigitalGauge.hpp
    PvInstrumentPanelObject.hpp
    PvInstrumentPanelObject.cpp
    PvInstrumentPanelGenerator.hpp
    PvInstrumentPanelGenerator.cpp
)

target_include_directories(
    ${PV_LIB_NAME} 
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} 
    $ENV{DXSDK_DIR}/Include
    ${CMAKE_SOURCE_DIR}/thirdParty/toml11/single_include
)

target_link_libraries(
    ${PV_LIB_NAME}
    PRIVATE 
    BVETS_LIBRARY_OPTIONS
)
