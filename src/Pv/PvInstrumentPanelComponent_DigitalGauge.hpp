#pragma once
#include "PvEnvironmentInformationSubscriberBase.hpp"
#include "PvInstrumentPanelComponentBase.hpp"
#include "PvInstrumentPanelComponent_CircularMeterObjectBase.hpp"
#include "PvInstrumentPanelComponent_ParametricObjectBase.hpp"

class PvInstrumentPanelComponent_DigitalGauge : public PvInstrumentPanelComponent_CircularMeterObjectBase,
                                                public PvInstrumentPanelComponent_ParametricObjectBase,
                                                public PvInstrumentPanelComponentBase,
                                                public PvEnvironmentInformationSubscriberBase
{
public:
    PvInstrumentPanelComponent_DigitalGauge() = default;
    double GetStep() const { return _step; }

    void SetStep(const double step) { _step = step; }

protected:
    void DoDraw(PvGrContextHandle context) override;
    void EnvironmentInformation_OnUpdated(const PvBridgeUpdateInfo& updateInfo) override;
    void EnvironmentInformation_OnDoorStateChanged(const bool isClosed) override;
    void EnvironmentInformation_OnTractionControllerPositionChanged(const int position) override;
    void EnvironmentInformation_OnBrakeControllerPositionChanged(const int position) override;

private:
    double _step = 0.0;
};
