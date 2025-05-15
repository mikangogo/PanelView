#include "PvInstrumentPanelComponent_DigitalNumber.hpp"

namespace
{
    void DrawImage(PvGrContextHandle context, PvGrTextureHandle texture, const double intervalY, const int frameIndex, const double blendingRatio)
    {
        PvGrSize2D texSize;

        pvGrSetTexture(context, texture);
        pvGrGetTextureSize(texture, texSize);

        const auto color = PvGrColor(255, 255, 255, 255);
        const auto currentV1 = (frameIndex * intervalY) / texSize.Height;
        const auto currentV2 = ((frameIndex + 1) * intervalY) / texSize.Height;


        const auto x2 = static_cast<double>(texSize.Width), x1 = 0.0;
        const auto y1 = 0.0, y2 = intervalY;
        constexpr auto u1 = 0.0, u2 = 1.0;
        const auto v1 = currentV1, v2 = currentV2;

        const std::array positions = {
            PvGrVector3D({.X = x1, .Y = y1, .Z = 0.0}), PvGrVector3D({.X = x2, .Y = y2, .Z = 0.0}),
            PvGrVector3D({.X = x1, .Y = y2, .Z = 0.0}), PvGrVector3D({.X = x1, .Y = y1, .Z = 0.0}),
            PvGrVector3D({.X = x2, .Y = y1, .Z = 0.0}), PvGrVector3D({.X = x2, .Y = y2, .Z = 0.0})};
        const std::array uvs = {
            PvGrVector2D({.X = u1, .Y = v1}), PvGrVector2D({.X = u2, .Y = v2}), PvGrVector2D({.X = u1, .Y = v2}),
            PvGrVector2D({.X = u1, .Y = v1}), PvGrVector2D({.X = u2, .Y = v1}), PvGrVector2D({.X = u2, .Y = v2}),
        };

        const std::array vertices = {
            PvGrPositionData_3D(positions[0], uvs[0], color), PvGrPositionData_3D(positions[1], uvs[1], color),
            PvGrPositionData_3D(positions[2], uvs[2], color), PvGrPositionData_3D(positions[3], uvs[3], color),
            PvGrPositionData_3D(positions[4], uvs[4], color), PvGrPositionData_3D(positions[5], uvs[5], color),
        };

        pvGrDrawTriangles(context, vertices);
    }
} // namespace

void PvInstrumentPanelComponent_DigitalNumber::DoDraw(PvGrContextHandle context)
{
    auto daytimeImage = GetTexture(TextureUsage_Daytime);
    auto nighttimeImage = GetTexture(TextureUsage_Daytime);
    auto blendingRatio = 1.0;

    PvGrSize2D texSize;
    pvGrGetTextureSize(daytimeImage, texSize);
    const auto interval = GetInterval();

    const auto& position = GetPosition();
    const auto position3D = PvGrVector3D{.X = position.X, .Y = position.Y};

    const auto value = static_cast<int>(GetParameterAtomic());


    pvGrPushModelTransformStack(context, PvGrTransform::TranslationTransform(position3D));
    pvGrApplyModelTransformStack(context);

    if (nighttimeImage)
    {
        DrawImage(context, nighttimeImage, interval, value, 1.0);
    }

    if (daytimeImage)
    {
        DrawImage(context, daytimeImage, interval, value, blendingRatio);
    }

    pvGrPopModelTransformStack(context);
}

void PvInstrumentPanelComponent_DigitalNumber::EnvironmentInformation_OnUpdated(const PvBridgeUpdateInfo& updateInfo)
{
    SetParameterFromUpdateInfo(updateInfo);

    SetParameterAtomic(GetParameter());
}

void PvInstrumentPanelComponent_DigitalNumber::EnvironmentInformation_OnDoorStateChanged(const bool isClosed)
{
    SetParameterFromDoorState(isClosed);
}

void PvInstrumentPanelComponent_DigitalNumber::EnvironmentInformation_OnTractionControllerPositionChanged(
    const int position)
{
    SetParameterFromTractionControllerPosition(position);
}

void PvInstrumentPanelComponent_DigitalNumber::EnvironmentInformation_OnBrakeControllerPositionChanged(
    const int position)
{
    SetParameterFromBrakeControllerPosition(position);
}
