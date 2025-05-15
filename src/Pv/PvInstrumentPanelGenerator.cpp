#include "PvInstrumentPanelGenerator.hpp"

#include <algorithm>

namespace
{
    struct CachedTextureData
    {
        PvGrTextureHandle Texture;
        std::filesystem::path Path;
    };

    struct WorkData
    {
        std::vector<CachedTextureData> CachedTextures;
        const PvInstrumentPanelDocument& Document;

        WorkData(const PvInstrumentPanelDocument& document)
            : CachedTextures(),
              Document(document)
        {
        }

        bool FindSameTextureFromCache(const std::filesystem::path& path, PvGrTextureHandle& texture) const
        {
            for (auto& t : CachedTextures)
            {
                if (hash_value(t.Path) != hash_value(path))
                {
                    continue;
                }

                texture = t.Texture;
                return true;
            }

            return false;
        }

        void RegisterTextureToCache(const PvGrTextureHandle texture, const  std::filesystem::path& path)
        {
            CachedTextureData textureData = {texture, path};
            CachedTextures.push_back(textureData);
        }

        void TransferTexturesFromCache(std::vector<PvGrTextureHandle>& dst) const
        {
            for (auto& t : CachedTextures)
            {
                dst.push_back(t.Texture);
            }
        }
    };

    void GenerateTexture(
        PvGrContextHandle grContext,
        PvInstrumentPanelComponent_TextureMappingObjectBase& component,
        PvInstrumentPanelComponent_TextureMappingObjectBase::TextureUsage usage,
        const std::filesystem::path& sourcePath,
        WorkData& workData
    )
    {
        auto& documentPath = workData.Document.GetPath();
        std::filesystem::path path;

        if (sourcePath.is_relative())
        {
            path = documentPath.parent_path();
        }

        path /= sourcePath;

        PvGrTextureHandle cachedTexture = nullptr;
        if (workData.FindSameTextureFromCache(path, cachedTexture))
        {
            component.SetTexture(cachedTexture, usage);
            return;
        }

        PvPfFileInfo fileInfo;
        if (!pvPfIoOpen(path.u8string().c_str(), fileInfo))
        {
            return;
        }

        if (pvPfIoRead(fileInfo))
        {
            auto newTexture = pvGrGenerateTextureFromFile(grContext, fileInfo);
            component.SetTexture(newTexture, usage);
            workData.RegisterTextureToCache(newTexture, fileInfo.GetPath());
        }

        pvPfIoClose(fileInfo);
    }

    void GenerateThisObject(
        PvGrContextHandle grContext,
        PvInstrumentPanelComponent_This& thisObject,
        WorkData& workData
    )
    {
        auto& document = workData.Document;
        auto& thisNode = document.GetThis();

        {
            thisObject.SetResolution(thisNode.Resolution.Value);
            thisObject.SetLeftMax(thisNode.Left.Value);
            thisObject.SetTopMax(thisNode.Top.Value);
            thisObject.SetRightMax(thisNode.Right.Value);
            thisObject.SetBottomMax(thisNode.Bottom.Value);
            thisObject.SetCenter({thisNode.Center.Value[0], thisNode.Center.Value[1]});
            thisObject.SetOrigin({thisNode.Origin.Value[0], thisNode.Origin.Value[1]});
            thisObject.SetPerspective(thisNode.Perspective.Value);
        }
        {
            PvInstrumentPanelComponent_TextureMappingObjectBase& tmo = thisObject;

            if (!thisNode.DaytimeImage.Path.empty())
            {
                GenerateTexture(grContext, tmo,
                                PvInstrumentPanelComponent_TextureMappingObjectBase::TextureUsage_Daytime,
                                thisNode.DaytimeImage.Path, workData);
            }

            if (!thisNode.NighttimeImage.Path.empty())
            {
                GenerateTexture(grContext, tmo,
                                PvInstrumentPanelComponent_TextureMappingObjectBase::TextureUsage_Nighttime,
                                thisNode.NighttimeImage.Path, workData);
            }
        }
    }

    void GeneratePilotLampObject(
        PvGrContextHandle grContext,
        PvInstrumentPanelComponent_PilotLamp& pilotLampObject,
        const size_t nodeIndex,
        WorkData& workData
    )
    {
        auto& document = workData.Document;
        auto& pilotLampNode = document.GetPilotLamp(nodeIndex);

        pilotLampObject.SetSubject(pilotLampNode.Subject.SubjectKey);
        pilotLampObject.SetSubjectIndex(pilotLampNode.Subject.Index);
        pilotLampObject.SetPosition(pilotLampNode.Location.Value[0], pilotLampNode.Location.Value[1]);
        pilotLampObject.SetTransparentColor(pilotLampNode.TransparentColor.A, pilotLampNode.TransparentColor.R,
                                            pilotLampNode.TransparentColor.G, pilotLampNode.TransparentColor.B);
        pilotLampObject.SetRenderingOrder(pilotLampNode.Layer.Value);

        {
            PvInstrumentPanelComponent_TextureMappingObjectBase& tmo = pilotLampObject;

            if (!pilotLampNode.DaytimeImage.Path.empty())
            {
                GenerateTexture(grContext, tmo,
                                PvInstrumentPanelComponent_TextureMappingObjectBase::TextureUsage_Daytime,
                                pilotLampNode.DaytimeImage.Path, workData);
            }

            if (!pilotLampNode.NighttimeImage.Path.empty())
            {
                GenerateTexture(grContext, tmo,
                                PvInstrumentPanelComponent_TextureMappingObjectBase::TextureUsage_Nighttime,
                                pilotLampNode.NighttimeImage.Path, workData);
            }
        }
    }

    void GenerateNeedleObject(
        PvGrContextHandle grContext,
        PvInstrumentPanelComponent_Needle& needleObject,
        const size_t nodeIndex,
        WorkData& workData
    )
    {
        auto& document = workData.Document;
        auto& needleNode = document.GetNeedle(nodeIndex);

        needleObject.SetSubject(needleNode.Subject.SubjectKey);
        needleObject.SetSubjectIndex(needleNode.Subject.Index);
        needleObject.SetPosition(needleNode.Location.Value[0], needleNode.Location.Value[1]);
        needleObject.SetTransparentColor(needleNode.TransparentColor.A, needleNode.TransparentColor.R,
                                         needleNode.TransparentColor.G, needleNode.TransparentColor.B);
        needleObject.SetSurfaceColor(needleNode.Color.A, needleNode.Color.R, needleNode.Color.G, needleNode.Color.B); 
        needleObject.SetRenderingOrder(needleNode.Layer.Value);
        needleObject.SetRotationOrigin(needleNode.Origin.Value[0], needleNode.Origin.Value[1]);
        needleObject.SetRadius(needleNode.Radius.Value);
        needleObject.SetInitialAngle(needleNode.InitialAngle.Value[0]);
        needleObject.SetLastAngle(needleNode.LastAngle.Value[0]);
        needleObject.SetMinimumValue(needleNode.Minimum.Value);
        needleObject.SetMaximumValue(needleNode.Maximum.Value);
        needleObject.SetTilt(needleNode.Tilt.Value[0], needleNode.Tilt.Value[1]);
        needleObject.SetHasStopPin(needleNode.StopPin.Value);
        needleObject.SetNaturalFrequency(needleNode.NaturalFreq.Value);
        needleObject.SetDampingRatio(needleNode.DampingRatio.Value);

        {
            PvInstrumentPanelComponent_TextureMappingObjectBase& tmo = needleObject;

            if (!needleNode.DaytimeImage.Path.empty())
            {
                GenerateTexture(grContext, tmo,
                                PvInstrumentPanelComponent_TextureMappingObjectBase::TextureUsage_Daytime,
                                needleNode.DaytimeImage.Path, workData);
            }

            if (!needleNode.NighttimeImage.Path.empty())
            {
                GenerateTexture(grContext, tmo,
                                PvInstrumentPanelComponent_TextureMappingObjectBase::TextureUsage_Nighttime,
                                needleNode.NighttimeImage.Path, workData);
            }
        }
    }

    void GenerateDigitalGaugeObject(
        PvInstrumentPanelComponent_DigitalGauge& digitalGaugeObject,
        const size_t nodeIndex,
        WorkData& workData
    )
    {
        auto& document = workData.Document;
        auto& digitalGaugeNode = document.GetDigitalGauge(nodeIndex);

        digitalGaugeObject.SetSubject(digitalGaugeNode.Subject.SubjectKey);
        digitalGaugeObject.SetSubjectIndex(digitalGaugeNode.Subject.Index);
        digitalGaugeObject.SetPosition(digitalGaugeNode.Location.Value[0], digitalGaugeNode.Location.Value[1]);
        digitalGaugeObject.SetSurfaceColor(digitalGaugeNode.Color.A, digitalGaugeNode.Color.R,
                                           digitalGaugeNode.Color.G, digitalGaugeNode.Color.B);
        digitalGaugeObject.SetRenderingOrder(digitalGaugeNode.Layer.Value);
        digitalGaugeObject.SetRadius(digitalGaugeNode.Radius.Value);
        digitalGaugeObject.SetInitialAngle(digitalGaugeNode.InitialAngle.Value[0]);
        digitalGaugeObject.SetLastAngle(digitalGaugeNode.LastAngle.Value[0]);
        digitalGaugeObject.SetMinimumValue(digitalGaugeNode.Minimum.Value);
        digitalGaugeObject.SetMaximumValue(digitalGaugeNode.Maximum.Value);
        digitalGaugeObject.SetTilt(digitalGaugeNode.Tilt.Value[0], digitalGaugeNode.Tilt.Value[1]);
        digitalGaugeObject.SetStep(digitalGaugeNode.Step.Value);
    }

    void GenerateDigitalNumberObject(
        PvGrContextHandle grContext,
        PvInstrumentPanelComponent_DigitalNumber& digitalNumberObject,
        const size_t nodeIndex,
        WorkData& workData
    )
    {
        auto& document = workData.Document;
        auto& digitalNumberNode = document.GetDigitalNumber(nodeIndex);

        digitalNumberObject.SetSubject(digitalNumberNode.Subject.SubjectKey);
        digitalNumberObject.SetSubjectIndex(digitalNumberNode.Subject.Index);
        digitalNumberObject.SetPosition(digitalNumberNode.Location.Value[0], digitalNumberNode.Location.Value[1]);
        digitalNumberObject.SetTransparentColor(digitalNumberNode.TransparentColor.A, digitalNumberNode.TransparentColor.R,
                                                digitalNumberNode.TransparentColor.G, digitalNumberNode.TransparentColor.B);
        digitalNumberObject.SetRenderingOrder(digitalNumberNode.Layer.Value);
        digitalNumberObject.SetInterval(digitalNumberNode.Interval.Value);

        {
            PvInstrumentPanelComponent_TextureMappingObjectBase& tmo = digitalNumberObject;

            if (!digitalNumberNode.DaytimeImage.Path.empty())
            {
                GenerateTexture(grContext, tmo,
                                PvInstrumentPanelComponent_TextureMappingObjectBase::TextureUsage_Daytime,
                                digitalNumberNode.DaytimeImage.Path, workData);
            }

            if (!digitalNumberNode.NighttimeImage.Path.empty())
            {
                GenerateTexture(grContext, tmo,
                                PvInstrumentPanelComponent_TextureMappingObjectBase::TextureUsage_Nighttime,
                                digitalNumberNode.NighttimeImage.Path, workData);
            }
        }
    }
}


bool PvInstrumentPanelGenerator::DocumentToObject(
    PvGrContextHandle grContext,
    PvInstrumentPanelObject& object,
    const PvInstrumentPanelDocument& document
)
{
    auto workData = std::make_unique<WorkData>(document);

    {
        GenerateThisObject(grContext, object._this, *workData);
    }

    for (size_t i = 0; i < document.GetPilotLampCount(); ++i)
    {
        GeneratePilotLampObject(grContext, *object._pilotLamps.emplace_back(std::make_unique<PvInstrumentPanelComponent_PilotLamp>()), i, *workData);
    }

    for (size_t i = 0; i < document.GetNeedleCount(); ++i)
    {
        GenerateNeedleObject(grContext, *object._needles.emplace_back(std::make_unique<PvInstrumentPanelComponent_Needle>()), i, *workData);
    }

    for (size_t i = 0; i < document.GetDigitalGaugeCount(); ++i)
    {
        GenerateDigitalGaugeObject(*object._digitalGauges.emplace_back(std::make_unique<PvInstrumentPanelComponent_DigitalGauge>()), i, *workData);
    }

    for (size_t i = 0; i < document.GetDigitalNumberCount(); ++i)
    {
        GenerateDigitalNumberObject(grContext, *object._digitalNumbers.emplace_back(std::make_unique<PvInstrumentPanelComponent_DigitalNumber>()), i, *workData);
    }


    for (auto& element : object._pilotLamps)
    {
        object._sortedComponents.emplace_back(*element);
    }

    for (auto& element : object._needles)
    {
        object._sortedComponents.emplace_back(*element);
    }

    for (auto& element : object._digitalGauges)
    {
        object._sortedComponents.emplace_back(*element);
    }

    for (auto& element : object._digitalNumbers)
    {
        object._sortedComponents.emplace_back(*element);
    }

    std::ranges::sort(object._sortedComponents,
                      [](const PvInstrumentPanelComponentBase& a, const PvInstrumentPanelComponentBase& b)
                      { return a.GetRenderingOrder() < b.GetRenderingOrder(); });

    workData->TransferTexturesFromCache(object._textures);
    workData.reset();

    object.OnGenerated();

    return true;
}
