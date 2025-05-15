#pragma once

#include "PvGr_D3D9.hpp"

class PvInstrumentPanelComponent_CircularMeterObjectBase
{
public:
    PvInstrumentPanelComponent_CircularMeterObjectBase() = default;
    void SetSurfaceColor(const double a,const double r, const double g, const double b) { _surfaceColor = PvGrColor::FromArgbFp64(a, r, g, b); }
    void SetRotationOrigin(const double x, const double y) { _rotationOrigin = PvGrVector2D(x, y); }
    void SetTilt(const double x, const double y) { _tilt = PvGrVector2D(x, y); }
    void SetRadius(const double radius) { _radius = radius; }
    void SetInitialAngle(const double initialAngle) { _initialAngle = initialAngle; }
    void SetLastAngle(const double lastAngle) { _lastAngle = lastAngle; }
    void SetMinimumValue(const double minimumValue) { _minimumValue = minimumValue; }
    void SetMaximumValue(const double maximumValue) { _maximumValue = maximumValue; }
    void SetNaturalFrequency(const double naturalFrequency) { _naturalFrequency = naturalFrequency; }
    void SetDampingRatio(const double dampingRatio) { _dampingRatio = dampingRatio; }
    void SetHasStopPin(const bool hasStopPin) { _hasStopPin = hasStopPin; }

    const PvGrColor& GetSurfaceColor() const { return _surfaceColor; }
    const PvGrVector2D& GetRotationOrigin() const { return _rotationOrigin; }
    const PvGrVector2D& GetTilt() const { return _tilt; }
    double GetRadius() const { return _radius; }
    double GetInitialAngle() const { return _initialAngle; }
    double GetLastAngle() const { return _lastAngle; }
    double GetMinimumValue() const { return _minimumValue; }
    double GetMaximumValue() const { return _maximumValue; }
    double GetNaturalFrequency() const { return _naturalFrequency; }
    double GetDampingRatio() const { return _dampingRatio; }
    bool GetHasStopPin() const { return _hasStopPin; }

protected:

private:
    PvGrColor _surfaceColor = PvGrColor(255, 255, 255, 255);
    PvGrVector2D _rotationOrigin;
    PvGrVector2D _tilt;
    double _radius = 0.0;
    double _initialAngle = 0.0;
    double _lastAngle = 0.0;
    double _minimumValue = 0.0;
    double _maximumValue = 0.0;
    double _naturalFrequency = 1.0;
    double _dampingRatio = 1.0;
    bool _hasStopPin = false;
};
