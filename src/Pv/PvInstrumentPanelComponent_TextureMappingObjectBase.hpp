#pragma once

#include "PvGr_D3D9.hpp"
#include "PvPf_Windows.hpp"

#include <array>

class PvInstrumentPanelComponent_TextureMappingObjectBase
{
public:
    enum TextureUsage
    {
        TextureUsage_Daytime = 0,
        TextureUsage_Nighttime,
        TextureUsage_MaxCount,
    };

    PvInstrumentPanelComponent_TextureMappingObjectBase() = default;

    void SetTexture(const PvGrTextureHandle texture, TextureUsage usage)
    {
        if ((usage < TextureUsage_Daytime) || (usage >= TextureUsage_MaxCount))
        {
            pvPfThrowException("Failed: PvInstrumentPanelComponent_TextureMappingObjectBase::SetTexture");
            return;
        }


        _hasTransparentColor = false;
        _textures.at(usage) = texture;
    }
    void SetTransparentColor(double a, double r, double g, double b)
    {
        _transparentColor = PvGrColor::FromArgbFp64(a, r, g, b);
        _hasTransparentColor = true;
    }

    PvGrTextureHandle GetTexture(TextureUsage usage) const
    {
        if ((usage < TextureUsage_Daytime) || (usage >= TextureUsage_MaxCount))
        {
            pvPfThrowException("Failed: PvInstrumentPanelComponent_TextureMappingObjectBase::GetTexture");
            return nullptr;
        }


        return _textures.at(usage);
    }
    const PvGrColor& GetTransparentColor() const
    {
        return _transparentColor;
    }

private:
    std::array<PvGrTextureHandle, TextureUsage_MaxCount> _textures = {};
    PvGrColor _transparentColor = PvGrColor(0, 0, 0, 0);
    bool _hasTransparentColor = false;
};
