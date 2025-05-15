#include "PvInstrumentPanelComponent_This.hpp"

namespace
{
    void DrawImage(PvGrContextHandle context, PvGrTextureHandle texture, double blendingRatio)
    {
        PvGrSize2D texSize;

        pvGrSetTexture(context, texture);
        pvGrGetTextureSize(texture, texSize);

        pvGrDrawRectangle(context, 
            texSize.Width, 
            texSize.Height,
            PvGrColor::FromArgbFp64(blendingRatio, 1.0, 1.0, 1.0)
        );
    }
}

void PvInstrumentPanelComponent_This::DoDraw(PvGrContextHandle context)
{
    auto daytimeImage = GetTexture(TextureUsage_Daytime);
    auto nightTimeImage = GetTexture(TextureUsage_Nighttime);
    auto blendingRatio = 1.0;

    if (nightTimeImage)
    {
        DrawImage(context, nightTimeImage, 1.0);
    }

    if (daytimeImage)
    {
        DrawImage(context, daytimeImage, blendingRatio);
    }
}

void PvInstrumentPanelComponent_This::EnvironmentInformation_OnUpdated(const PvBridgeUpdateInfo& updateInfo)
{
}
