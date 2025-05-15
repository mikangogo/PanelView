#pragma once

#include "PvEnvironmentInformationSubscriberBase.hpp"
#include "PvInstrumentPanelComponentBase.hpp"
#include "PvInstrumentPanelComponent_TextureMappingObjectBase.hpp"

class PvInstrumentPanelComponent_This : public PvInstrumentPanelComponent_TextureMappingObjectBase,
                                        public PvInstrumentPanelComponentBase,
                                        public PvEnvironmentInformationSubscriberBase
{
public:
    PvInstrumentPanelComponent_This() = default;
    void SetResolution(double resolution) { _resolution = resolution; }
    void SetLeftMax(double leftMax) { _leftMax = leftMax; }
    void SetTopMax(double topMax) { _topMax = topMax; }
    void SetRightMax(double rightMax) { _rightMax = rightMax; }
    void SetBottomMax(double bottomMax) { _bottomMax = bottomMax; }
    void SetCenter(const PvGrVector2D& center) { _center = center; }
    void SetOrigin(const PvGrVector2D& origin) { _origin = origin; }
    void SetPerspective(double perspective) { _perspective = perspective; }

    double GetResolution() const { return _resolution; }
    double GetLeftMax() const { return _leftMax; }
    double GetTopMax() const { return _topMax; }
    double GetRightMax() const { return _rightMax; }
    double GetBottomMax() const { return _bottomMax; }
    PvGrVector2D GetCenter() const { return _center; }
    PvGrVector2D GetOrigin() const { return _origin; }
    double GetPerspective() const { return _perspective; }

protected:
    void DoDraw(PvGrContextHandle context) override;
    void EnvironmentInformation_OnUpdated(const PvBridgeUpdateInfo& updateInfo) override;

private:
    double _resolution = 0.0;
    double _leftMax = 0.0;
    double _topMax = 0.0;
    double _rightMax = 0.0;
    double _bottomMax = 0.0;
    PvGrVector2D _center;
    PvGrVector2D _origin;
    double _perspective = 0.0;
};
