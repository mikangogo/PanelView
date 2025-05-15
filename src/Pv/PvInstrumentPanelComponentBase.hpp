#pragma once

#include "PvGr_D3D9.hpp"

class PvInstrumentPanelComponentBase
{
public:
    PvInstrumentPanelComponentBase() = default;
    virtual ~PvInstrumentPanelComponentBase() = default;

    void SetPosition(const double x, const double y)
    {
        _position.X = x;
        _position.Y = y;
    }

    void SetRenderingOrder(double renderingOrder) { _renderingOrder = renderingOrder; }

    void SetDeltaTime(const double deltaTime)
    {
        _deltaTime = deltaTime;
    }

    void SetParameterAtomic(double value)
    {
        _parameterAtomic.store(value, std::memory_order::relaxed);
    }


    const PvGrVector2D& GetPosition() const
    {
        return _position;
    }

    double GetRenderingOrder() const
    {
        return _renderingOrder;
    }

    double GetDeltaTime() const
    {
        return _deltaTime;
    }

    double GetParameterAtomic() const
    {
        return _parameterAtomic.load(std::memory_order::relaxed);
    }


    void Draw(PvGrContextHandle context) { DoDraw(context); }

protected:
    virtual void DoDraw(PvGrContextHandle context) = 0;

private:
    PvGrVector2D _position;
    PvGrTransform _transformMatrix;
    double _renderingOrder = 0.0;
    double _deltaTime;
    std::atomic<double> _parameterAtomic;
};
