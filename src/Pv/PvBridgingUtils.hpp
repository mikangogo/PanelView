#pragma once

#include "PvBridgingDefinitions.h"

inline const PvBridgeSpecInfo& pvBridgeToSpecInfo(const void* eventInfo)
{
    return *static_cast<const PvBridgeSpecInfo*>(eventInfo);
}

inline const PvBridgeBeaconInfo& pvBridgeToBeaconInfo(const void* eventInfo)
{
    return *static_cast<const PvBridgeBeaconInfo*>(eventInfo);
}

inline const PvBridgeUpdateInfo& pvBridgeToUpdateInfo(const void* eventInfo)
{
    return *static_cast<const PvBridgeUpdateInfo*>(eventInfo);
}

inline int pvBridgeToTractionControllerPosition(const void* eventInfo)
{
    return *static_cast<const int*>(eventInfo);
}

inline int pvBridgeToBrakeControllerPosition(const void* eventInfo)
{
    return *static_cast<const int*>(eventInfo);
}

inline int pvBridgeToReverserPosition(const void* eventInfo)
{
    return *static_cast<const int*>(eventInfo);
}

inline int pvBridgeToButtonIndex(const void* eventInfo)
{
    return *static_cast<const int*>(eventInfo);
}

inline int pvBridgeToHornIndex(const void* eventInfo)
{
    return *static_cast<const int*>(eventInfo);
}

inline int pvBridgeToSignalIndex(const void* eventInfo)
{
    return *static_cast<const int*>(eventInfo);
}
