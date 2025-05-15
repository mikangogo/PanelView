#include "PvInstrumentPanelComponent_PilotLamp.hpp"

namespace
{
    void DrawImage(PvGrContextHandle context,  PvGrTextureHandle texture, double blendingRatio)
    {
        PvGrSize2D texSize;

        pvGrSetTexture(context, texture);
        pvGrGetTextureSize(texture, texSize);

        pvGrDrawRectangle(context, texSize.Width, texSize.Height,
                          PvGrColor::FromArgbFp64(blendingRatio, 1.0, 1.0, 1.0));
    }
} // namespace

void PvInstrumentPanelComponent_PilotLamp::DoDraw(PvGrContextHandle context)
{
    auto daytimeImage = GetTexture(TextureUsage_Daytime);
    auto nightTimeImage = GetTexture(TextureUsage_Nighttime);
    auto blendingRatio = 1.0;

    auto& position = GetPosition();
    auto position3D = PvGrVector3D(position.X, position.Y);

    pvGrPushModelTransformStack(context, PvGrTransform::TranslationTransform(position3D));
    pvGrApplyModelTransformStack(context);

    if (GetParameterAtomic() != 0.0)
    {
        if (nightTimeImage)
        {
            DrawImage(context, nightTimeImage, 1.0);
        }

        if (daytimeImage)
        {
            DrawImage(context, daytimeImage, blendingRatio);
        }
    }

    pvGrPopModelTransformStack(context);
}

void PvInstrumentPanelComponent_PilotLamp::EnvironmentInformation_OnUpdated(const PvBridgeUpdateInfo& updateInfo)
{
    SetParameterFromUpdateInfo(updateInfo);
    SetParameterAtomic(GetParameter());
}

void PvInstrumentPanelComponent_PilotLamp::EnvironmentInformation_OnDoorStateChanged(const bool isClosed)
{
    SetParameterFromDoorState(isClosed);
}

void PvInstrumentPanelComponent_PilotLamp::EnvironmentInformation_OnTractionControllerPositionChanged(
    const int position)
{
    SetParameterFromTractionControllerPosition(position);
}

void PvInstrumentPanelComponent_PilotLamp::EnvironmentInformation_OnBrakeControllerPositionChanged(const int position)
{
    SetParameterFromBrakeControllerPosition(position);
}
