#include "PvEnvironmentInformationPublisher.hpp"

#include "PvBridgingUtils.hpp"

namespace
{

}

PvEnvironmentInformationPublisher::PvEnvironmentInformationPublisher()  
{  
}

PvEnvironmentInformationPublisher::~PvEnvironmentInformationPublisher()
{
}

void PvEnvironmentInformationPublisher::PublishEvent(PvBridgeEventType eventType, const void* eventInfo)
{
    std::shared_lock lock(_subscriberListGuard);

    switch (eventType)
    {
        case PvEventType_OnSpecDecided:
            {
                const auto& specInfo = pvBridgeToSpecInfo(eventInfo);

                for (const auto& subscriber : _subscribers)
                {
                    subscriber.get().EnvironmentInformation_OnSpecDecided(specInfo);
                }
            }
            break;
        case PvEventType_OnTractionControllerPositionChanged:
            {
                const auto tcPos = pvBridgeToTractionControllerPosition(eventInfo);

                for (const auto& subscriber : _subscribers)
                {
                    subscriber.get().EnvironmentInformation_OnTractionControllerPositionChanged(tcPos);
                }
            }
            break;
        case PvEventType_OnBrakePControllerPositionChanged:
            {
                const auto bcPos = pvBridgeToBrakeControllerPosition(eventInfo);

                for (const auto& subscriber : _subscribers)
                {
                    subscriber.get().EnvironmentInformation_OnBrakeControllerPositionChanged(bcPos);
                }
            }
            break;
        case PvEventType_OnReverserPositionChanged:
            {
                const auto rvPos = pvBridgeToReverserPosition(eventInfo);

                for (const auto& subscriber : _subscribers)
                {
                    subscriber.get().EnvironmentInformation_OnReverserPositionChanged(rvPos);
                }
            }
            break;
        case PvEventType_OnBeaconReceived:
            {
                const auto& beaconInfo = pvBridgeToBeaconInfo(eventInfo);

                for (const auto& subscriber : _subscribers)
                {
                    subscriber.get().EnvironmentInformation_OnBeaconReceived(beaconInfo);
                }
            }
            break;
        case PvEventType_OnButtonPressing:
            {
                const auto buttonIndex = pvBridgeToButtonIndex(eventInfo);

                for (const auto& subscriber : _subscribers)
                {
                    subscriber.get().EnvironmentInformation_OnButtonPressing(buttonIndex);
                }
            }
            break;
        case PvEventType_OnButtonReleased:
            {
                const auto buttonIndex = pvBridgeToButtonIndex(eventInfo);

                for (const auto& subscriber : _subscribers)
                {
                    subscriber.get().EnvironmentInformation_OnButtonReleased(buttonIndex);
                }
            }
            break;
        case PvEventType_OnHornBlew:
            {
                const auto hornIndex = pvBridgeToHornIndex(eventInfo);

                for (const auto& subscriber : _subscribers)
                {
                    subscriber.get().EnvironmentInformation_OnHornBlow(hornIndex);
                }
            }
            break;
        case PvEventType_OnDoorClosed:
            {
                for (const auto& subscriber : _subscribers)
                {
                    subscriber.get().EnvironmentInformation_OnDoorStateChanged(true);
                }
            }
            break;
        case PvEventType_OnDoorOpened:
            {
                for (const auto& subscriber : _subscribers)
                {
                    subscriber.get().EnvironmentInformation_OnDoorStateChanged(false);
                }
            }
            break;
        case PvEventType_OnSignalChanged:
            {
                for (const auto& subscriber : _subscribers)
                {
                    const auto& beaconData = pvBridgeToBeaconInfo(eventInfo);

                    subscriber.get().EnvironmentInformation_OnSignalStateChanged(false);
                }
            }
            break;
    }

    // unlock()
}

void PvEnvironmentInformationPublisher::PublishStartRunning(const int param)
{
    std::shared_lock lock(_subscriberListGuard);

    for (const auto& subscriber : _subscribers)
    {
        subscriber.get().EnvironmentInformation_OnRunningStarted(param);
    }

    // unlock()
}

void PvEnvironmentInformationPublisher::PublishUpdate(const PvBridgeUpdateInfo& updateInfo)
{
    std::shared_lock lock(_subscriberListGuard);

    for (const auto& subscriber : _subscribers)
    {
        subscriber.get().EnvironmentInformation_OnUpdated(updateInfo);
    }

    // unlock()
}


void PvEnvironmentInformationPublisher::RegisterSubscriber(PvEnvironmentInformationSubscriberBase& subscriber)
{
    std::lock_guard lock(_subscriberListGuard);

    _subscribers.emplace_back(subscriber);

    // unlock()
}

void PvEnvironmentInformationPublisher::RemoveSubscriber(const PvEnvironmentInformationSubscriberBase& subscriber)
{
    std::lock_guard lock(_subscriberListGuard);

    const auto subscriberPointer = &subscriber;

    for (auto iter = _subscribers.begin(); iter != _subscribers.end(); ++iter)
    {
        const auto pointer = &iter->get();

        if (subscriberPointer != pointer)
        {
            continue;
        }

        _subscribers.erase(iter);
        break;
    }

    // unlock()
}
