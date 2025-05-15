#pragma once

#include "PvBridgingDefinitions.h"

class PvEnvironmentInformationSubject
{
public:
    long long CurrentFrame;

    PvBridgeSpecInfo SpecInfo;
    PvBridgeBeaconInfo BeaconInfo;
    PvBridgeUpdateInfo UpdateInfo;

    int TractionControllerPosition;
    int BrakeControllerPosition;
    int ReverserPosition;
};
