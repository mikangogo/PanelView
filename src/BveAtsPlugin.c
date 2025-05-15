#include "BveAtsPlugin.h"

#include <stddef.h>

#include "Pv/PvMain.h"


// Called when this plug-in is loaded
ATS_API void WINAPI Load(void)
{
    EXPORT_THIS_FUNCTION
    pvStartUp();
}

// Called when this plug-in is unloaded
ATS_API void WINAPI Dispose(void)
{
    EXPORT_THIS_FUNCTION
    pvShutdown();
}

// Returns the version numbers of ATS plug-in
ATS_API int WINAPI GetPluginVersion(void)
{
    EXPORT_THIS_FUNCTION

    return ATS_VERSION;
}

// Called when the train is loaded
ATS_API void WINAPI SetVehicleSpec(ATS_VEHICLESPEC vehicleSpecification)
{
    EXPORT_THIS_FUNCTION

    PvBridgeSpecInfo specInfo;

    specInfo.Cars = vehicleSpecification.Cars;
    specInfo.PowerNotches = vehicleSpecification.PowerNotches;
    specInfo.BrakeNotches = vehicleSpecification.BrakeNotches;
    specInfo.AtsNotch = vehicleSpecification.AtsNotch;
    specInfo.B67Notch = vehicleSpecification.B67Notch;

    pvHandleEvent(PvEventType_OnSpecDecided, &specInfo);
}

// Called when the game is started
ATS_API void WINAPI Initialize(int initialHandlePosition)
{
    EXPORT_THIS_FUNCTION
    pvStartRunning(initialHandlePosition);
}

// Called every frame
ATS_API ATS_HANDLES WINAPI Elapse(ATS_VEHICLESTATE vehicleStatus, int* panelStatus, int* soundStatus)
{
    EXPORT_THIS_FUNCTION

    PvBridgeOutputInfo outputInfo;
    PvBridgeUpdateInfo updateInfo;
    ATS_HANDLES handles;

    updateInfo.Location = vehicleStatus.Location;
    updateInfo.Speed = vehicleStatus.Speed;
    updateInfo.Time = vehicleStatus.Time;
    updateInfo.BcPressure = vehicleStatus.BcPressure;
    updateInfo.MrPressure = vehicleStatus.MrPressure;
    updateInfo.ErPressure = vehicleStatus.ErPressure;
    updateInfo.BpPressure = vehicleStatus.BpPressure;
    updateInfo.SapPressure = vehicleStatus.SapPressure;
    updateInfo.Current = vehicleStatus.Current;
    updateInfo.PanelArray = panelStatus;
    updateInfo.SoundArray = soundStatus;

    pvUpdate(&outputInfo, &updateInfo);

    handles.Power = outputInfo.Power;
    handles.Brake = outputInfo.Brake;
    handles.Reverser = outputInfo.Reverser;
    handles.ConstantSpeed = outputInfo.ConstantSpeed;

    return handles;
}

// Called when the power is changed
ATS_API void WINAPI SetPower(int tractionControlHandlePosition)
{
    EXPORT_THIS_FUNCTION
    pvHandleEvent(PvEventType_OnTractionControllerPositionChanged, &tractionControlHandlePosition);
}

// Called when the brake is changed
ATS_API void WINAPI SetBrake(int brakeControlHandlePosition)
{
    EXPORT_THIS_FUNCTION
    pvHandleEvent(PvEventType_OnBrakePControllerPositionChanged, &brakeControlHandlePosition);
}

// Called when the reverser is changed
ATS_API void WINAPI SetReverser(int reverserHandlePosition)
{
    EXPORT_THIS_FUNCTION
    pvHandleEvent(PvEventType_OnReverserPositionChanged, &reverserHandlePosition);
}

// Called when any ATS key is pressed
ATS_API void WINAPI KeyDown(int key)
{
    EXPORT_THIS_FUNCTION
    pvHandleEvent(PvEventType_OnButtonPressing, &key);
}

// Called when any ATS key is released
ATS_API void WINAPI KeyUp(int key)
{
    EXPORT_THIS_FUNCTION
    pvHandleEvent(PvEventType_OnButtonReleased, &key);
}

// Called when the horn is used
ATS_API void WINAPI HornBlow(int hornType)
{
    EXPORT_THIS_FUNCTION
    pvHandleEvent(PvEventType_OnHornBlew, &hornType);
}

// Called when the door is opened
ATS_API void WINAPI DoorOpen(void)
{
    EXPORT_THIS_FUNCTION
    pvHandleEvent(PvEventType_OnDoorOpened, NULL);
}

// Called when the door is closed
ATS_API void WINAPI DoorClose(void)
{
    EXPORT_THIS_FUNCTION
    pvHandleEvent(PvEventType_OnDoorClosed, NULL);
}

// Called when current signal is changed
ATS_API void WINAPI SetSignal(int signal)
{
    EXPORT_THIS_FUNCTION
    pvHandleEvent(PvEventType_OnSignalChanged, &signal);
}

// Called when the beacon data is received
ATS_API void WINAPI SetBeaconData(ATS_BEACONDATA receivedBeaconData)
{
    EXPORT_THIS_FUNCTION

    PvBridgeBeaconInfo beaconInfo;

    beaconInfo.Type = receivedBeaconData.Type;
    beaconInfo.Signal = receivedBeaconData.Signal;
    beaconInfo.Distance = receivedBeaconData.Distance;
    beaconInfo.Optional = receivedBeaconData.Optional;

    pvHandleEvent(PvEventType_OnBeaconReceived, &beaconInfo);
}