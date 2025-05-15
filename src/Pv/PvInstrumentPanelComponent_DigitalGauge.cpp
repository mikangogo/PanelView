#include "PvInstrumentPanelComponent_DigitalGauge.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace
{
    double GetSteppedValue(const double in, const double step)
    {
        return std::floor(std::abs(in / step)) * step * (std::signbit(in) ? -1.0 : 1.0);
    }

    int GetOrthant(double degree)
    {
        if (degree == 0.0)
        {
            return 0;
        }

        return static_cast<int>(((abs(degree) / 45.0) + 1.0) * (signbit(degree) ? -1.0 : 1.0));
    }

    void CalculateRectanglePosition(PvGrVector2D& out, const double angleDegree)
    {
        auto angle = PvGrToRadian(angleDegree);
        auto sinTheta = sin(angle);
        auto cosTheta = cos(angle);

        out.X = cosTheta / (std::max)(abs(cosTheta), abs(sinTheta));
        out.Y = sinTheta / (std::max)(abs(cosTheta), abs(sinTheta));
    }

    void DrawSquareFromTheta(PvGrContextHandle context, const double currentAngle, const double beginAngle, const double endAngle, const PvGrColor& color)
    {
        PvGrVector2D currentPosition;
        PvGrVector2D beginPosition;
        PvGrVector2D endPosition;
        CalculateRectanglePosition(currentPosition, currentAngle);
        CalculateRectanglePosition(beginPosition, beginAngle);
        CalculateRectanglePosition(endPosition, endAngle);

        const auto currentOrthant = GetOrthant(currentAngle);
        constexpr std::array positions = {
            // {1}
            PvGrVector3D({.X = 0.0, .Y = 0.0}),
            PvGrVector3D({.X = 1.0, .Y = 0.0}),
            PvGrVector3D({.X = 1.0, .Y = 1.0}),

            // {2}
            PvGrVector3D({.X = 0.0, .Y = 0.0}),
            PvGrVector3D({.X = 1.0, .Y = 1.0}),
            PvGrVector3D({.X = 0.0, .Y = 1.0}),

            // {3}
            PvGrVector3D({.X = 0.0, .Y = 0.0}),
            PvGrVector3D({.X = 0.0, .Y = 1.0}),
            PvGrVector3D({.X = -1.0, .Y = 1.0}),

            // {4}
            PvGrVector3D({.X = 0.0, .Y = 0.0}),
            PvGrVector3D({.X = -1.0, .Y = 1.0}),
            PvGrVector3D({.X = -1.0, .Y = 0.0}),


            // {-4}
            PvGrVector3D({.X = 0.0, .Y = 0.0}),
            PvGrVector3D({.X = -1.0, .Y = 0.0}),
            PvGrVector3D({.X = -1.0, .Y = -1.0}),

            // {-3}
            PvGrVector3D({.X = 0.0, .Y = 0.0}),
            PvGrVector3D({.X = -1.0, .Y = -1.0}),
            PvGrVector3D({.X = 0.0, .Y = -1.0}),

            // {-2}
            PvGrVector3D({.X = 0.0, .Y = 0.0}),
            PvGrVector3D({.X = 0.0, .Y = -1.0}),
            PvGrVector3D({.X = 1.0, .Y = -1.0}),

            // {-1}
            PvGrVector3D({.X = 0.0, .Y = 0.0}),
            PvGrVector3D({.X = 1.0, .Y = -1.0}),
            PvGrVector3D({.X = 1.0, .Y = 0.0}),
        };

        for (auto i = 0; i < 8; ++i)
        {
            constexpr auto orthants = std::array{1, 2, 3, 4, -4, -3, -2, -1};
            const auto orthant = orthants[i];
            constexpr std::array uvs = {
                PvGrVector2D(),
                PvGrVector2D(),
                PvGrVector2D(),
            };

            std::array vertices = {
                PvGrPositionData_3D(positions[0 + (i * 3)], uvs[0], color),
                PvGrPositionData_3D(positions[1 + (i * 3)], uvs[1], color),
                PvGrPositionData_3D(positions[2 + (i * 3)], uvs[2], color),
            };

            if (currentOrthant == orthant)
            {
                vertices[1] = PvGrPositionData_3D(PvGrVector3D({currentPosition.X, currentPosition.Y}), uvs[1], color);
            }
            else
            {
                if (currentAngle > (45.0 * (orthant < 0 ? orthant : orthant - 1)))
                {
                    continue;
                }
            }

            pvGrDrawTriangles(context, vertices);
        }
    }

}

void PvInstrumentPanelComponent_DigitalGauge::DoDraw(PvGrContextHandle context)
{
    const auto& position = GetPosition();
    const auto position3D = PvGrVector3D(position.X, position.Y, 0.0);
    const auto& tilt = GetTilt();
    const auto radius = GetRadius();

    pvGrSetTexture(context, nullptr);

    pvGrPushModelTransformStack(context, PvGrTransform::TranslationTransform(position3D));
    pvGrPushModelTransformStack(context, PvGrTransform::RotationYTransform(PvGrToRadian(tilt.Y)));
    pvGrPushModelTransformStack(context, PvGrTransform::RotationXTransform(PvGrToRadian(-tilt.X)));
    pvGrPushModelTransformStack(context, PvGrTransform::ScalingTransform(PvGrVector3D({radius, radius, 1.0})));
    pvGrPushModelTransformStack(context, PvGrTransform::RotationZTransform(PvGrToRadian(-90.0)));
    pvGrApplyModelTransformStack(context);

    DrawSquareFromTheta(context, GetParameterAtomic(), 0.0, 0.0, GetSurfaceColor());

    pvGrPopModelTransformStack(context);
    pvGrPopModelTransformStack(context);
    pvGrPopModelTransformStack(context);
    pvGrPopModelTransformStack(context);
    pvGrPopModelTransformStack(context);
}

void PvInstrumentPanelComponent_DigitalGauge::EnvironmentInformation_OnUpdated(const PvBridgeUpdateInfo& updateInfo)
{
    SetParameterFromUpdateInfo(updateInfo);

    const auto initialAngle = GetInitialAngle();
    const auto lastAngle = GetLastAngle();
    const auto minimumValue = GetMinimumValue();
    const auto maximumValue = GetMaximumValue();

    auto currentAngle = GetSteppedValue(
        GetParameter() * ((lastAngle - initialAngle) / (maximumValue - minimumValue)) + initialAngle, GetStep());

    SetParameterAtomic(currentAngle);
}

void PvInstrumentPanelComponent_DigitalGauge::EnvironmentInformation_OnDoorStateChanged(const bool isClosed)
{
    SetParameterFromDoorState(isClosed);
}

void PvInstrumentPanelComponent_DigitalGauge::EnvironmentInformation_OnTractionControllerPositionChanged(
    const int position)
{
    SetParameterFromTractionControllerPosition(position);
}

void PvInstrumentPanelComponent_DigitalGauge::EnvironmentInformation_OnBrakeControllerPositionChanged(
    const int position)
{
    SetParameterFromBrakeControllerPosition(position);
}
