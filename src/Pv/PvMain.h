#ifndef PV_PVMAIN_H_INCLUDED
#define PV_PVMAIN_H_INCLUDED

#include "PvBridgingDefinitions.h"

#if defined(__cplusplus)
extern "C"
{
#endif

extern void pvStartUp(void);
extern void pvShutdown(void);
extern void pvStartRunning(int initializeType);
extern void pvHandleEvent(PvBridgeEventType eventType, const void* eventInfo);
extern void pvUpdate(PvBridgeOutputInfo* outputInfo, const PvBridgeUpdateInfo* updateInfo);

#if defined(__cplusplus)
}
#endif

#endif  // PV_PVMAIN_H_INCLUDED
