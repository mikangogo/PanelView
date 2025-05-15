#pragma once

#include "PvBridgingDefinitions.h"

class PvEnvironmentInformationSubscriberBase
{
    friend class PvEnvironmentInformationPublisher;
public:
    PvEnvironmentInformationSubscriberBase() = default;
    virtual ~PvEnvironmentInformationSubscriberBase() = default;
    PvEnvironmentInformationSubscriberBase(const PvEnvironmentInformationSubscriberBase& other) = default;
    PvEnvironmentInformationSubscriberBase(PvEnvironmentInformationSubscriberBase&& other) noexcept = default;
    PvEnvironmentInformationSubscriberBase& operator=(const PvEnvironmentInformationSubscriberBase& other) = default;
    PvEnvironmentInformationSubscriberBase& operator=(PvEnvironmentInformationSubscriberBase&& other) noexcept = default;

protected:
    virtual void EnvironmentInformation_OnSpecDecided(const PvBridgeSpecInfo& specInfo);

    virtual void EnvironmentInformation_OnTractionControllerPositionChanged(const int position);
    virtual void EnvironmentInformation_OnBrakeControllerPositionChanged(const int position);
    virtual void EnvironmentInformation_OnReverserPositionChanged(const int position);

    virtual void EnvironmentInformation_OnDoorStateChanged(const bool isClosed);
    virtual void EnvironmentInformation_OnHornBlow(const int hornIndex);

    virtual void EnvironmentInformation_OnButtonPressing(const int buttonIndex);
    virtual void EnvironmentInformation_OnButtonReleased(const int buttonIndex);

    virtual void EnvironmentInformation_OnBeaconReceived(const PvBridgeBeaconInfo& beaconInfo);
    virtual void EnvironmentInformation_OnSignalStateChanged(const int signalIndex);

    virtual void EnvironmentInformation_OnRunningStarted(const int param);
    virtual void EnvironmentInformation_OnUpdated(const PvBridgeUpdateInfo& updateInfo) = 0;
};
