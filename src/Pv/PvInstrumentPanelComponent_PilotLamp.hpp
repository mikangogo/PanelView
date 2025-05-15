#pragma once
#include "PvEnvironmentInformationSubscriberBase.hpp"
#include "PvInstrumentPanelComponentBase.hpp"
#include "PvInstrumentPanelComponent_ParametricObjectBase.hpp"
#include "PvInstrumentPanelComponent_TextureMappingObjectBase.hpp"

class PvInstrumentPanelComponent_PilotLamp : public PvInstrumentPanelComponent_TextureMappingObjectBase,
                                             public PvInstrumentPanelComponent_ParametricObjectBase,
                                             public PvInstrumentPanelComponentBase,
                                             public PvEnvironmentInformationSubscriberBase

{
protected:
    void DoDraw(PvGrContextHandle context) override;
    void EnvironmentInformation_OnUpdated(const PvBridgeUpdateInfo& updateInfo) override;
    void EnvironmentInformation_OnDoorStateChanged(const bool isClosed) override;
    void EnvironmentInformation_OnTractionControllerPositionChanged(const int position) override;
    void EnvironmentInformation_OnBrakeControllerPositionChanged(const int position) override;

};
