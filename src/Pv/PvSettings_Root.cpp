#include "PvSettings_Root.hpp"

#include "PvPf_Windows.hpp"
#include "PvSettingsTomlUtils.hpp"
#include "PvStringUtils.hpp"

std::span<const std::reference_wrapper<PvSettings_Window>> PvSettings_Root::GetWindowSettings()
{
    return _windowSettingsReferences;
}

bool PvSettings_Root::OnFileLoading(const std::filesystem::path& filePath)
{
    if (!pvSettingsTomlLoadDocumentFromFile(filePath,
                                       [&](const std::span<unsigned char>& doc) { SetDocument(doc); }))
    {
        return false;
    }


    const auto result = toml::try_parse_str<PvSettingsTomlU8Config>(GetDocument().data());

    if (result.is_err())
    {
        pvPfDebugPrintLine(
            std::format("PvSettings_Root: Load(): Parse failed: {}",
                pvStringFmtAnsi(filePath.u8string().c_str()))
        );

        return false;
    }

    auto directoryPath = filePath.parent_path();

    auto& root = result.unwrap();

    if (!pvSettingsTomlIsValidVersion(root, u8"PanelViewSettings 1.0"))
    {
        return false;
    }


    auto baseDirectoryNode = pvSettingsTomlGetString(root, u8"BaseDirectory");

    auto windowNode = pvSettingsTomlGetNode(root, u8"Window");

    if (!windowNode)
    {
        return false;
    }

    for (auto i = 1; ; ++i)
    {
        auto s = std::format("{:d}", i);
        auto node = pvSettingsTomlGetNode(windowNode.value(), pvStringFmtU8(s.c_str()));

        if (!node)
        {
            break;
        }

        auto panelFileNode = pvSettingsTomlGetString(node.value(), u8"PanelFile", false, true);
        auto settingFileNode = pvSettingsTomlGetString(node.value(), u8"SettingFile");
        auto backgroundColorNode = pvSettingsTomlGetInteger(node.value(), u8"BackgroundColorRgb");

        if (!panelFileNode)
        {
            break;
        }

        PvSettings_Window::WindowRootSettings windowRootSettings;

        {
            auto panelFilePath = std::filesystem::path(panelFileNode.value());
            pvStringMakeAbsolutePath(panelFilePath, directoryPath);
            windowRootSettings.PanelFilePath = panelFilePath;
        }

        if (settingFileNode)
        {
            auto settingsFilePath = std::filesystem::path(settingFileNode.value());
            pvStringMakeAbsolutePath(settingsFilePath, directoryPath);
            windowRootSettings.SettingsFilePath = settingsFilePath;
        }

        if (backgroundColorNode)
        {
            auto r = static_cast<unsigned char>(0xFF & (backgroundColorNode.value() >> 16));
            auto g = static_cast<unsigned char>(0xFF & (backgroundColorNode.value() >> 8));
            auto b = static_cast<unsigned char>(0xFF & (backgroundColorNode.value() >> 0));
            windowRootSettings.BackgroundColor = PvGrColor::FromArgb(0xFF, r, g, b);
        }


        auto windowSettings = std::make_unique<PvSettings_Window>(windowRootSettings);


        if (settingFileNode)
        {
            auto settingsFilePath = std::filesystem::path(settingFileNode.value());

            pvStringMakeAbsolutePath(settingsFilePath, directoryPath);

            windowSettings->Load(settingsFilePath);
        }

        auto& ref = *_windowSettingsInstances.emplace_back(std::move(windowSettings));
        _windowSettingsReferences.emplace_back(ref);
    }

    return true;
}
