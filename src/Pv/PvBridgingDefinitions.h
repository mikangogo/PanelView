#ifndef PV_PVBRIDGEDEFINITIONS_H_INCLUDED
#define PV_PVBRIDGEDEFINITIONS_H_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct
{
    int BrakeNotches; // Number of Brake Notches
    int PowerNotches; // Number of Power Notches
    int AtsNotch; // ATS Cancel Notch
    int B67Notch; // 80% Brake (67 degree)
    int Cars; // Number of Cars
} PvBridgeSpecInfo;

typedef struct
{
    int Type; // Type of Beacon
    int Signal; // Signal of Connected Section
    float Distance; // Distance to Connected Section (m)
    int Optional; // Optional Data
} PvBridgeBeaconInfo;

typedef struct
{
    int Brake; // Brake Notch
    int Power; // Power Notch
    int Reverser; // Reverser Position
    int ConstantSpeed; // Constant Speed Control
} PvBridgeOutputInfo;

typedef struct
{
    double Location; // Train Position (Z-axis) (m)
    float Speed; // Train Speed (km/h)
    int Time; // Time (ms)
    float BcPressure; // Pressure of Brake Cylinder (Pa)
    float MrPressure; // Pressure of MR (Pa)
    float ErPressure; // Pressure of ER (Pa)
    float BpPressure; // Pressure of BP (Pa)
    float SapPressure; // Pressure of SAP (Pa)
    float Current; // Current (A)
    int* PanelArray;
    int* SoundArray;
} PvBridgeUpdateInfo;

typedef enum
{
    PvEventType_OnSpecDecided = 0,
    PvEventType_OnTractionControllerPositionChanged,
    PvEventType_OnBrakePControllerPositionChanged,
    PvEventType_OnReverserPositionChanged,
    PvEventType_OnBeaconReceived,
    PvEventType_OnButtonPressing,
    PvEventType_OnButtonReleased,
    PvEventType_OnHornBlew,
    PvEventType_OnDoorClosed,
    PvEventType_OnDoorOpened,
    PvEventType_OnSignalChanged,
} PvBridgeEventType;

#if defined(__cplusplus)
}
#endif

#endif // PV_PVBRIDGEDEFINITIONS_H_INCLUDED
