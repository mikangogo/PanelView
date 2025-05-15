#pragma once

#include "PvSettingsBase.hpp"

#include <filesystem>

#include "PvGr_D3D9.hpp"

class PvSettings_Window : public PvSettingsBase
{
public:
    struct WindowRootSettings
    {
        std::filesystem::path PanelFilePath;
        std::filesystem::path SettingsFilePath;
        PvGrColor BackgroundColor = PvGrColor::FromArgb(255, 0, 0, 0);
    };

    struct WindowPlacementSettings
    {
        PvPfWindowRectangle WindowRect;
        bool IsMaximized;
        bool IsTopmost;
        bool IsBorderless;
    };

    virtual ~PvSettings_Window() = default;

    explicit PvSettings_Window(const WindowRootSettings& windowRootSettings) : _windowRootSettings(windowRootSettings)
    {
    }

    const PvGrColor& GetBackgroundColor() const;
    const std::filesystem::path& GetPanelFilePath() const;
    const std::optional<WindowPlacementSettings>& GetWindowPlacementSettings() const;

    void Store(WindowPlacementSettings& settings);

protected:
    bool OnFileLoading(const std::filesystem::path& filePath) override;

private:
    WindowRootSettings _windowRootSettings;
    std::optional<WindowPlacementSettings> _windowPlacementSettings = std::nullopt;
};
