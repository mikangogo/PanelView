#include "PvInstrumentPanelObject.hpp"

#include "PvEnvironmentInformationPublisher.hpp"

namespace
{
    void AdaptAspectRatio(PvGrViewPort vp, PvGrVector2D& plane)
    {
        const auto aspectRatio = vp.Width / vp.Height;

        if (aspectRatio >= 1.0)
        {
            plane.Y = 0.5 / aspectRatio;
        }
        else
        {
            plane.X = 0.5 * aspectRatio;
        }
    }

    void ApplyZooming(double& clippingPlaneScaling, PvGrVector2D& clippingPlaneOffset, const PvGrVector2D& targetRect,
                      const PvGrRectangle2D& boundingRect, const PvGrVector2D& clippingPlaneSize, const double scaling)
    {
        const auto previousScaling = clippingPlaneScaling;

        if (scaling != 0.0)
        {
            clippingPlaneScaling = (std::min)(clippingPlaneScaling + scaling, PvInstrumentPanelObject::MaxScaling);
        }


        clippingPlaneScaling = (std::max)(clippingPlaneScaling,
                                   2.0 *
                                       max(clippingPlaneSize.X / (boundingRect.Right() - boundingRect.Left()),
                                           clippingPlaneSize.Y / (boundingRect.Top() - boundingRect.Bottom())));

        clippingPlaneOffset += targetRect * ((1.0 / previousScaling) - (1.0 / clippingPlaneScaling));
    }


    void AdaptZooming(double& clippingPlaneScaling, PvGrVector2D& clippingPlaneOffset,
                      const PvGrRectangle2D& boundingRect, const PvGrVector2D& clippingPlaneSize,
                      const double value)
    {
        const auto previousScaling = clippingPlaneScaling;
        const auto previousOffset = -clippingPlaneOffset * clippingPlaneScaling;

        clippingPlaneScaling = (std::max)(clippingPlaneScaling,
                   2.0 *
                           (std::max)(clippingPlaneSize.X / (boundingRect.Right() - boundingRect.Left()),
                           clippingPlaneSize.Y / (boundingRect.Top() - boundingRect.Bottom())));

        clippingPlaneOffset += previousOffset * ((1.0 / previousScaling) - (1.0 / clippingPlaneScaling));
    }

    void ClampClippingPlaneOffset(PvGrVector2D& clippingPlaneOffset, const PvGrRectangle2D& boundingRect,
                                                           const PvGrVector2D& clippingPlaneSize, const double clippingPlaneScaling)
    {
        const auto sw = clippingPlaneSize.X / clippingPlaneScaling;
        if (clippingPlaneOffset.X < boundingRect.Left() + sw)
        {
            clippingPlaneOffset.X = boundingRect.Left() + sw;
        }
        else if (clippingPlaneOffset.X > boundingRect.Right() - sw)
        {
            clippingPlaneOffset.X = boundingRect.Right() - sw;
        }

        const auto sh = clippingPlaneSize.Y / clippingPlaneScaling;
        if (clippingPlaneOffset.Y < boundingRect.Bottom() + sh)
        {
            clippingPlaneOffset.Y = boundingRect.Bottom() + sh;
        }
        else if (clippingPlaneOffset.Y > boundingRect.Top() - sh)
        {
            clippingPlaneOffset.Y = boundingRect.Top() - sh;
        }
    }

}

PvInstrumentPanelObject::PvInstrumentPanelObject(): _this(), _pilotLamps(), _needles(), _digitalNumbers()
{
}

PvInstrumentPanelObject::~PvInstrumentPanelObject()
{
    for (auto& t : _textures)
    {
        pvGrDeleteTexture(t);
    }
}

void PvInstrumentPanelObject::OnMouseWheel(int delta, int mousePositionX, int mousePositionY)
{
    _mouseDelta = delta;
    _mousePositionX = mousePositionX;
    _mousePositionY = mousePositionY;
}

void PvInstrumentPanelObject::OnMouseDown(int mousePositionX, int mousePositionY)
{
    _isMouseDragging = true;
    _mousePositionX = mousePositionX;
    _mousePositionY = mousePositionY;
    _lastMousePositionX = mousePositionX;
    _lastMousePositionY = mousePositionY;
}

void PvInstrumentPanelObject::OnMouseDragging(int mousePositionX, int mousePositionY)
{
    _mousePositionX = mousePositionX;
    _mousePositionY = mousePositionY;
}

void PvInstrumentPanelObject::OnMouseUp(int mousePositionX, int mousePositionY)
{
    _isMouseDragging = false;
}

PvGrRectangle2D PvInstrumentPanelObject::MakeClippingPlane(PvGrContextHandle context)
{
    const auto origin = _this.GetOrigin();
    const auto resolution = _this.GetResolution();
    const auto divResolution = 1.0 / resolution;


    const auto boundingArea = PvGrRectangle2D(
        {
            .X = _this.GetLeftMax(),
            .Y = _this.GetTopMax(),
            .Width = _this.GetRightMax() - _this.GetLeftMax(),
            .Height = _this.GetBottomMax() - _this.GetTopMax()
        }
    );


    PvGrViewPort vp = {};
    pvGrGetViewport(context, vp);

    auto clippingPlaneSize = PvGrVector2D({.X = 0.5, .Y = 0.5});
    AdaptAspectRatio(vp, clippingPlaneSize);


    auto& clippingPlaneOffset = _clippingPlaneOffset;

    {
        const auto boundingRect = PvGrRectangle2D({.X = (boundingArea.Left() - origin.X) * divResolution,
            .Y = (-(boundingArea.Top() - origin.Y)) * divResolution,
            .Width = boundingArea.Width * divResolution,
            .Height = -(boundingArea.Height) * divResolution});
        const auto longSide = 1.0 / (std::max)(vp.Width, vp.Height);

        if (_mouseDelta != 0.0)
        {
            constexpr auto ratio = 1.0 / 1000.0;
            auto target =
                PvGrVector2D((_mousePositionX - vp.Width * 0.5), -(_mousePositionY - vp.Height * 0.5)) * longSide;
            ApplyZooming(_clippingPlaneScaling, clippingPlaneOffset, target, boundingRect, clippingPlaneSize,
                         _mouseDelta * ratio);
        }

        if (_isMouseDragging)
        {
            auto moveVector =
                PvGrVector2D(_lastMousePositionX - _mousePositionX, -(_lastMousePositionY - _mousePositionY)) * longSide / _clippingPlaneScaling;

            clippingPlaneOffset += moveVector;

            _lastMousePositionX = _mousePositionX;
            _lastMousePositionY = _mousePositionY;
        }

        AdaptZooming(_clippingPlaneScaling, clippingPlaneOffset, boundingRect, clippingPlaneSize, 0.0);
        ClampClippingPlaneOffset(clippingPlaneOffset, boundingRect, clippingPlaneSize, _clippingPlaneScaling);

        _mouseDelta *= ZoomingSensitivity;
    }

    {
        const auto cps = 1.0 / _clippingPlaneScaling;
        const auto clippingPlane = PvGrRectangle2D({.X = clippingPlaneOffset.X - (clippingPlaneSize.X * cps),
            .Y = clippingPlaneOffset.Y + (clippingPlaneSize.Y * cps),
            .Width = (2.0 * clippingPlaneSize.X) * cps,
            .Height = (-2.0 * clippingPlaneSize.Y) * cps});
        return clippingPlane;
    }
}

void PvInstrumentPanelObject::Draw(PvGrContextHandle context, double deltaTime)
{

    {
        const auto clippingPlane = MakeClippingPlane(context);
        PvGrPerspectiveOffCenterProjectionTransform pm;
        pm.Left = clippingPlane.Left() * 0.4;
        pm.Right = clippingPlane.Right() * 0.4;
        pm.Bottom = clippingPlane.Bottom() * 0.4;
        pm.Top = clippingPlane.Top() * 0.4;
        pm.NearZ = 0.4;
        pm.FarZ = 1.6;
        pm.ApplyLeftHand();
        pvGrSetProjectionTransform(context, pm);
    }

    pvGrSetViewTransform(context, PvGrLookAtViewTransform::Identity());

    pvGrClearModelTransformStack(context);

    {
        const auto origin = _this.GetOrigin();
        const auto resolution = _this.GetResolution();
        const auto divResolution = 1.0 / resolution;

        pvGrPushModelTransformStack(context, PvGrTransform::TranslationTransform({0.0, 0.0, 0.8}));
        pvGrPushModelTransformStack(context, PvGrTransform::ScalingTransform(PvGrVector3D(0.8, 0.8, 0.8) * divResolution));
        pvGrPushModelTransformStack(context, PvGrTransform::TranslationTransform({-origin.X, origin.Y, 0.6}));
        pvGrPushModelTransformStack(context, PvGrTransform::ScalingTransform({1.0, -1.0, 1.0}));

        pvGrApplyModelTransformStack(context);
    }


    _this.SetDeltaTime(deltaTime);
    _this.Draw(context);

    for (PvInstrumentPanelComponentBase& sortedComponent : _sortedComponents)
    {
        sortedComponent.SetDeltaTime(deltaTime);
        sortedComponent.Draw(context);
    }


    pvGrPopModelTransformStack(context);
    pvGrPopModelTransformStack(context);
    pvGrPopModelTransformStack(context);
    pvGrPopModelTransformStack(context);
}

void PvInstrumentPanelObject::SubscribeEnvironmentInformation(PvEnvironmentInformationPublisher& publisher) const
{
    for (auto& pilotLamp : _pilotLamps)
    {
        publisher.RegisterSubscriber(*pilotLamp);
    }

    for (auto& needle : _needles)
    {
        publisher.RegisterSubscriber(*needle);
    }

    for (auto& digitalGauge : _digitalGauges)
    {
        publisher.RegisterSubscriber(*digitalGauge);
    }

    for (auto& digitalNumber : _digitalNumbers)
    {
        publisher.RegisterSubscriber(*digitalNumber);
    }
}

void PvInstrumentPanelObject::UnsubscribeEnvironmentInformation(PvEnvironmentInformationPublisher& publisher) const
{
    for (auto& pilotLamp : _pilotLamps)
    {
        publisher.RemoveSubscriber(*pilotLamp);
    }

    for (auto& needle : _needles)
    {
        publisher.RemoveSubscriber(*needle);
    }

    for (auto& digitalGauge : _digitalGauges)
    {
        publisher.RemoveSubscriber(*digitalGauge);
    }

    for (auto& digitalNumber : _digitalNumbers)
    {
        publisher.RemoveSubscriber(*digitalNumber);
    }
}

void PvInstrumentPanelObject::OnGenerated()
{
    const auto origin = _this.GetOrigin();
    const auto center = _this.GetCenter();
    const auto divResolution = 1.0 / _this.GetResolution();

    _clippingPlaneOffset =
        PvGrVector2D({.X = (center.X - origin.X) * divResolution, .Y = (-(center.Y - origin.Y)) * divResolution});
}

