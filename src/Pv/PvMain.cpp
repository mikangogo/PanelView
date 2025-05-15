#include "PvMain.h"

#include <memory>
#include <vector>

#include "PvGr_D3D9.hpp"
#include "PvPanelWindow.hpp"
#include "PvSettings_Root.hpp"

#include "PvEnvironmentInformationPublisher.hpp"
#include "PvBridgingUtils.hpp"

namespace
{
    struct PanelInstance
    {
        PanelInstance() = default;
        ~PanelInstance() = default;
        PanelInstance(PanelInstance& c) = delete;
        PanelInstance(PanelInstance&& c) noexcept
        {
            Window = std::move(c.Window);
            Document = std::move(c.Document);
        }
        PanelInstance& operator=(PanelInstance&&) = delete;
        PanelInstance& operator=(PanelInstance&) = delete;


        void Delete()
        {
            Window->Delete();
            Window.reset();
            Document.reset();
        }

        std::unique_ptr<PvPanelWindow> Window;
        std::unique_ptr<PvInstrumentPanelDocument> Document;
    };

    std::vector<PanelInstance> Panels;

    std::unique_ptr<PvEnvironmentInformationPublisher> EnvironmentInfoPublisher;

    PvSettings_Root RootSettings;

    PvBridgeOutputInfo OutputInfo; 

    bool CreatePanel(PanelInstance& panelInstance, const char8_t* panelFilePath)
    {
        if (panelInstance.Window || panelInstance.Document)
        {
            return false;
        }


        PvPfFileInfo fileInfo;
        auto failed = [](PvPfFileInfo& fileInfo) -> bool
        {
            pvPfIoClose(fileInfo);
            return false;
        };

        if (!pvPfIoOpen(panelFilePath, fileInfo))
        {
            return false;
        }

        if (!pvPfIoRead(fileInfo))
        {
            return failed(fileInfo);
        }

        auto document = std::make_unique<PvInstrumentPanelDocument>();

        const auto loadResult = PvInstrumentPanelDocument::LoadFromFile(fileInfo, *document);

        pvPfIoClose(fileInfo);

        if (!loadResult)
        {
            return false;
        }

        panelInstance.Document = std::move(document);
        panelInstance.Window = std::make_unique<PvPanelWindow>();
        panelInstance.Window->Create(*panelInstance.Document, *EnvironmentInfoPublisher);

        return true;
    }
}

void pvStartUp()
{
    pvGrInitialize();

    auto rootSettingsPath =
#if defined(_PV_DEVELOPMENT_BUILD)  
        u8"..\\..\\testing\\data\\panelview.toml";
#else
        u8"panelview.toml";
#endif

    if (!RootSettings.Load(rootSettingsPath))
    {
        pvPfThrowException("Failed: pvStartUp(). Failed loading the \"panelview.toml\".");
        return;
    }

    EnvironmentInfoPublisher = std::make_unique<PvEnvironmentInformationPublisher>();


    auto windowSettingsArray = RootSettings.GetWindowSettings();

    for (auto windowSettings : windowSettingsArray)
    {
        PanelInstance panel;
        auto& settings = windowSettings.get();

        if (CreatePanel(panel, settings.GetPanelFilePath().u8string().c_str()))
        {
            panel.Window->Setup(settings);
            panel.Window->SetVisible(true);
            Panels.emplace_back(std::move(panel));
        }
    }

    OutputInfo = {.Brake = 0, .Power = 0, .Reverser = 0, .ConstantSpeed = 0};
}

void pvShutdown()
{
    for (auto& panel : Panels)
    {
        panel.Delete();
    }

    EnvironmentInfoPublisher.reset();

    pvGrShutdown();
}

void pvHandleEvent(PvBridgeEventType eventType, const void* eventInfo)
{
    EnvironmentInfoPublisher->PublishEvent(eventType, eventInfo);

    switch (eventType)
    {
        case PvEventType_OnTractionControllerPositionChanged:
            OutputInfo.Power = pvBridgeToTractionControllerPosition(eventInfo);
            break;
        case PvEventType_OnBrakePControllerPositionChanged:
            OutputInfo.Brake = pvBridgeToBrakeControllerPosition(eventInfo);
            break;
        case PvEventType_OnReverserPositionChanged:
            OutputInfo.Reverser = pvBridgeToReverserPosition(eventInfo);
            break;
        default:
            break;
    }
}

void pvUpdate(PvBridgeOutputInfo* outputInfo, const PvBridgeUpdateInfo* updateInfo)
{
    EnvironmentInfoPublisher->PublishUpdate(*updateInfo);
    *outputInfo = OutputInfo;
}

void pvStartRunning(int initializeType)
{
    EnvironmentInfoPublisher->PublishStartRunning(initializeType);
}
