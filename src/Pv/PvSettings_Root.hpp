#pragma once

#include <span>
#include <filesystem>

#include "PvSettingsBase.hpp"
#include "PvSettings_Window.hpp"

class PvSettings_Root : public PvSettingsBase
{
public:
    virtual ~PvSettings_Root() = default;
    explicit PvSettings_Root() = default;

    const std::filesystem::path& GetBaseDirectory();

    std::span<const std::reference_wrapper<PvSettings_Window>> GetWindowSettings();

protected:
    bool OnFileLoading(const std::filesystem::path& filePath) override;

private:
    std::vector<std::unique_ptr<PvSettings_Window>> _windowSettingsInstances;
    std::vector<std::reference_wrapper<PvSettings_Window>> _windowSettingsReferences;
};
