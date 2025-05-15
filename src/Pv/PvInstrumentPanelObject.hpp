#pragma once
#include <vector>

#include "PvInstrumentPanelComponent_DigitalGauge.hpp"
#include "PvInstrumentPanelComponent_DigitalNumber.hpp"
#include "PvInstrumentPanelComponent_Needle.hpp"
#include "PvInstrumentPanelComponent_PilotLamp.hpp"
#include "PvInstrumentPanelComponent_This.hpp"

class PvInstrumentPanelObject
{
    friend class PvInstrumentPanelGenerator;

public:
    PvInstrumentPanelObject();
    ~PvInstrumentPanelObject();

    void OnMouseWheel(int delta, int mousePositionX, int mousePositionY);
    void OnMouseDown(int mousePositionX, int mousePositionY);
    void OnMouseDragging(int mousePositionX, int mousePositionY);
    void OnMouseUp(int mousePositionX, int mousePositionY);

    void Draw(PvGrContextHandle context, double deltaTime);

    void SubscribeEnvironmentInformation(PvEnvironmentInformationPublisher& publisher) const;
    void UnsubscribeEnvironmentInformation(PvEnvironmentInformationPublisher& publisher) const;

    static constexpr double MaxScaling = 100.0;
    static constexpr double ZoomingSensitivity = 0.5;

private:
    void OnGenerated();

    PvGrRectangle2D MakeClippingPlane(PvGrContextHandle context);


    PvInstrumentPanelComponent_This _this;
    std::vector<std::unique_ptr<PvInstrumentPanelComponent_PilotLamp>> _pilotLamps;
    std::vector<std::unique_ptr<PvInstrumentPanelComponent_Needle>> _needles;
    std::vector<std::unique_ptr<PvInstrumentPanelComponent_DigitalGauge>> _digitalGauges;
    std::vector<std::unique_ptr<PvInstrumentPanelComponent_DigitalNumber>> _digitalNumbers;

    std::vector<PvGrTextureHandle> _textures;

    std::vector<std::reference_wrapper<PvInstrumentPanelComponentBase>> _sortedComponents;


    double _clippingPlaneScaling = 1.0;
    PvGrVector2D _clippingPlaneOffset = {};

    double _mouseDelta = 0.0;
    int _mousePositionX = 0;
    int _mousePositionY = 0;

    bool _isMouseDragging = false;
    int _lastMousePositionX = 0;
    int _lastMousePositionY = 0;
};
