#include "PvSettings_Window.hpp"

#include "PvPf_Windows.hpp"
#include "PvSettingsTomlUtils.hpp"
#include "PvStringUtils.hpp"

const PvGrColor& PvSettings_Window::GetBackgroundColor() const
{
    return _windowRootSettings.BackgroundColor;
}

const std::filesystem::path& PvSettings_Window::GetPanelFilePath() const
{
    return _windowRootSettings.PanelFilePath;
}

const std::optional<PvSettings_Window::WindowPlacementSettings>& PvSettings_Window::GetWindowPlacementSettings() const
{
    return _windowPlacementSettings;
}


void PvSettings_Window::Store(WindowPlacementSettings& settings)
{
    std::u8string s;

    {
        toml::basic_value<PvSettingsTomlU8Config> versionNode(toml::basic_value<PvSettingsTomlU8Config>::table_type{
            {u8"Version", u8"PanelViewWindowSettings 1.0"},
        });
        s += toml::format<PvSettingsTomlU8Config>(versionNode);
    }

    {
        s += u8"# This document is AUTO-GENERATED.\n\n";
    }

    {
        toml::basic_value<PvSettingsTomlU8Config> placementNode(toml::basic_value<PvSettingsTomlU8Config>::table_type
        {
            {u8"Left", settings.WindowRect.Left},
            {u8"Right", settings.WindowRect.Right},
            {u8"Top", settings.WindowRect.Top},
            {u8"Bottom", settings.WindowRect.Bottom},
            {u8"Maximized", settings.IsMaximized},
            {u8"Topmost", settings.IsTopmost},
            {u8"Borderless", settings.IsBorderless},
        });

        s += toml::format<PvSettingsTomlU8Config>(u8"Placement", placementNode);
    }

    if (!_windowRootSettings.SettingsFilePath.empty())
    {
        PvPfFileInfo fileInfo;

        if (pvPfIoOpen(_windowRootSettings.SettingsFilePath.u8string().c_str(), fileInfo, PvPfIoOpenControlFlag_Write | PvPfIoOpenControlFlag_CreateNew))
        {
            pvPfIoExpandData(fileInfo, s.size());
            std::ranges::copy(s, fileInfo.GetData().data());
            pvPfIoWrite(fileInfo);
            pvPfIoClose(fileInfo);
        }
    }

    _windowPlacementSettings = settings;
}

bool PvSettings_Window::OnFileLoading(const std::filesystem::path& filePath)
{
    _windowRootSettings.SettingsFilePath = filePath;


    if (!pvSettingsTomlLoadDocumentFromFile(filePath, [&](const std::span<unsigned char>& doc) { SetDocument(doc); }))
    {
        return false;
    }

    const auto result = toml::try_parse_str<PvSettingsTomlU8Config>(GetDocument().data());

    if (result.is_err())
    {
        pvPfDebugPrintLine(
            std::format("PvSettings_Window: Load(): Parse failed: {}", pvStringFmtAnsi(filePath.u8string().c_str())));

        return false;
    }

    auto& root = result.unwrap();

    if (!pvSettingsTomlIsValidVersion(root, u8"PanelViewWindowSettings 1.0"))
    {
        return false;
    }

    auto placementNode = pvSettingsTomlGetNode(root, u8"Placement");

    if (placementNode)
    {
        const auto leftNode = pvSettingsTomlGetInteger(placementNode.value(), u8"Left", false, true);
        const auto rightNode = pvSettingsTomlGetInteger(placementNode.value(), u8"Right", false, true);
        const auto topNode = pvSettingsTomlGetInteger(placementNode.value(), u8"Top", false, true);
        const auto bottomNode = pvSettingsTomlGetInteger(placementNode.value(), u8"Bottom", false, true);
        const auto maximizedNode = pvSettingsTomlGetBoolean(placementNode.value(), u8"Maximized", false, true);
        const auto topmostNode = pvSettingsTomlGetBoolean(placementNode.value(), u8"Topmost", false, true);
        const auto borderlessNode = pvSettingsTomlGetBoolean(placementNode.value(), u8"Borderless", false, true);

        if (!leftNode || !rightNode || !topNode || !bottomNode || !maximizedNode || !topmostNode || !borderlessNode)
        {
            pvPfDebugPrintLine(std::format("PvSettings_Window: Load(): Parse failed: {}",
                                           pvStringFmtAnsi(filePath.u8string().c_str())));

            return false;
        }

        WindowPlacementSettings settings = {
            PvPfWindowRectangle(static_cast<long>(leftNode.value()),
                                static_cast<long>(rightNode.value()),
                                static_cast<long>(topNode.value()),
                                static_cast<long>(bottomNode.value())),
            maximizedNode.value(),
            topmostNode.value(),
            borderlessNode.value()
        };

        _windowPlacementSettings = settings;
    }

    return true;
}
