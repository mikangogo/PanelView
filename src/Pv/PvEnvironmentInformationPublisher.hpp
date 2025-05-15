#pragma once

#include <memory>
#include <shared_mutex>
#include <vector>

#include "PvBridgingDefinitions.h"
#include "PvEnvironmentInformationSubscriberBase.hpp"

class PvEnvironmentInformationPublisher
{
public:
    PvEnvironmentInformationPublisher();
    ~PvEnvironmentInformationPublisher();

    void PublishEvent(PvBridgeEventType eventType, const void* eventInfo);
    void PublishStartRunning(const int param);
    void PublishUpdate(const PvBridgeUpdateInfo& updateInfo);

    void RegisterSubscriber(PvEnvironmentInformationSubscriberBase& subscriber);
    void RemoveSubscriber(const PvEnvironmentInformationSubscriberBase& subscriber);


private:
    long long _currentFrame = 0LL;
    std::vector<std::reference_wrapper<PvEnvironmentInformationSubscriberBase>> _subscribers;
    std::shared_mutex _subscriberListGuard;
};
