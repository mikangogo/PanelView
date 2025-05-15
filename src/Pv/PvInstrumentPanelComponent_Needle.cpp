#include "PvInstrumentPanelComponent_Needle.hpp"

#include <algorithm>

namespace
{
    void DrawImage(PvGrContextHandle context, PvGrTextureHandle texture, double blendingRatio, const PvGrColor& color)
    {
        PvGrSize2D texSize;

        pvGrSetTexture(context, texture);
        pvGrGetTextureSize(texture, texSize);

        pvGrDrawRectangle(context, texSize.Width, texSize.Height,
                          PvGrColor::FromArgbFp64(blendingRatio, color.R, color.G, color.B));
    }

    double GetSteppedValue(const double in, const double step)
    {
        return std::floor(std::abs(in / step)) * step * (std::signbit(in) ? -1.0 : 1.0);
    }

    double CalculateDampedSpringSystem(double& v, double& x, const double target, const double k, const double c, const double dt)
    {
        const double omega = k; // std::sqrt(k); // guess: sqrt(k/m) ???
        const double a = -2.0 * c * omega * v   // damping term
            - (omega * omega) * (x - target)    // spring term
            + 0.0;   // external force

        v += a * dt;
        x += v * dt;

        return x;
    }
} // namespace

void PvInstrumentPanelComponent_Needle::DoDraw(PvGrContextHandle context)
{
    auto daytimeImage = GetTexture(TextureUsage_Daytime);
    auto nightTimeImage = GetTexture(TextureUsage_Nighttime);
    const auto blendingRatio = 1.0;

    const auto& position = GetPosition();
    const auto position3D = PvGrVector3D(position.X, position.Y, 0.0);

    const auto& origin = GetRotationOrigin();
    const auto origin3D = PvGrVector3D(-origin.X, -origin.Y, 0.0);

    const auto scale = abs(GetRadius() / origin3D.Y);
    const auto& tilt = GetTilt();


    pvGrPushModelTransformStack(context, PvGrTransform::TranslationTransform(position3D));
    pvGrPushModelTransformStack(context, PvGrTransform::RotationYTransform(PvGrToRadian(tilt.Y)));
    pvGrPushModelTransformStack(context, PvGrTransform::RotationXTransform(PvGrToRadian(-tilt.X)));
    pvGrPushModelTransformStack(context, PvGrTransform::RotationZTransform(PvGrToRadian(GetParameterAtomic())));
    pvGrPushModelTransformStack(context, PvGrTransform::ScalingTransform({scale, scale, 1.0}));
    pvGrPushModelTransformStack(context, PvGrTransform::TranslationTransform(origin3D));

    pvGrApplyModelTransformStack(context);

    if (nightTimeImage)
    {
        DrawImage(context, nightTimeImage, 1.0, GetSurfaceColor());
    }

    if (daytimeImage)
    {
        DrawImage(context, daytimeImage, blendingRatio, GetSurfaceColor());

    }

    pvGrPopModelTransformStack(context);
    pvGrPopModelTransformStack(context);
    pvGrPopModelTransformStack(context);
    pvGrPopModelTransformStack(context);
    pvGrPopModelTransformStack(context);
    pvGrPopModelTransformStack(context);
}

void PvInstrumentPanelComponent_Needle::EnvironmentInformation_OnUpdated(const PvBridgeUpdateInfo& updateInfo)
{
    SetParameterFromUpdateInfo(updateInfo);

    const auto initialAngle = GetInitialAngle();
    const auto lastAngle = GetLastAngle();
    const auto direction = initialAngle < lastAngle ? 1 : -1;
    const auto minimumValue = GetMinimumValue();
    const auto maximumValue = GetMaximumValue();

    const auto stopPin = GetHasStopPin();


    const auto ratio = ((lastAngle - initialAngle) / (maximumValue - minimumValue));
    auto currentAngle = GetParameter() * ratio - (minimumValue * ratio - initialAngle);

    if (stopPin)
    {
        if (direction)
        {
            currentAngle = (std::min)(lastAngle, currentAngle);
            currentAngle = (std::max)(initialAngle, currentAngle);
        }
        else
        {
            currentAngle = (std::max)(lastAngle, currentAngle);
            currentAngle = (std::min)(initialAngle, currentAngle);
        }
    }

    auto naturalFreq = GetNaturalFrequency();
    auto angle = 0.0;

    if (naturalFreq > 0.0)
    {
        const auto maxTime = static_cast<int>(GetDeltaTime() * 1000.0);

        for (auto i = 0; i < maxTime; ++i)
        {
            constexpr auto deltaTime = 1.0 / 1000.0;
            angle =
                CalculateDampedSpringSystem(_v, _x, currentAngle, GetNaturalFrequency(), GetDampingRatio(), deltaTime);
        }

        if (stopPin)
        {
            if (direction)
            {
                angle = (std::min)(lastAngle, angle);
                angle = (std::max)(initialAngle, angle);
            }
            else
            {
                angle = (std::max)(lastAngle, angle);
                angle = (std::min)(initialAngle, angle);
            }
        }
    }
    else
    {
        angle = currentAngle;
    }

    SetParameterAtomic(angle);
}

void PvInstrumentPanelComponent_Needle::EnvironmentInformation_OnDoorStateChanged(const bool isClosed)
{
    SetParameterFromDoorState(isClosed);
}

void PvInstrumentPanelComponent_Needle::EnvironmentInformation_OnTractionControllerPositionChanged(const int position)
{
    SetParameterFromTractionControllerPosition(position);
}

void PvInstrumentPanelComponent_Needle::EnvironmentInformation_OnBrakeControllerPositionChanged(const int position)
{
    SetParameterFromBrakeControllerPosition(position);
}
