#include "PvSettingsTomlUtils.hpp"

#include "PvPf_Windows.hpp"
#include "PvStringUtils.hpp"

bool pvSettingsTomlLoadDocumentFromFile(const std::filesystem::path& filePath,
                                        const std::function<void(const std::span<unsigned char>&)>& setDocumentMethod)
{
    PvPfFileInfo fi;

    if (!pvPfIoOpen(filePath.u8string().c_str(), fi))
    {
        return false;
    }

    if (!pvPfIoRead(fi))
    {
        pvPfDebugPrintLine(std::format("PvSettingsToml: File not found: {}",
                                       pvStringFmtAnsi(filePath.u8string().c_str())));
        pvPfIoClose(fi);
        return false;
    }

    setDocumentMethod(fi.GetData());

    pvPfIoClose(fi);

    return true;
}

bool pvSettingsTomlValidateKey(const toml::basic_value<PvSettingsTomlU8Config>& toml, const char8_t* key)
{
    if (!toml.contains(key))
    {
        pvPfDebugPrintLine(std::format("PvSettingsToml: Missing key: {}", pvStringFmtAnsi(key)));
        return false;
    }

    return true;
}

bool pvSettingsTomlValidateValue(const toml::basic_value<PvSettingsTomlU8Config>& toml, const char8_t* key, const toml::value_t type,
    const bool allowEmpty)
{
    if (!toml.contains(key))
    {
        return false;
    }


    auto node = toml.at(key);


    if (node.is_empty() && !allowEmpty)
    {
        pvPfDebugPrintLine(std::format("PvSettingsToml: Key: {}: Value is empty: Expected is {}", pvStringFmtAnsi(key),
                                       toml::to_string(type)));
        return false;
    }

    if (!node.is(type) && !allowEmpty)
    {
        pvPfDebugPrintLine(std::format("PvSettingsToml: Key: {}: Invalid value type: Expected is {}, actual is {}",
                                       pvStringFmtAnsi(key), toml::to_string(type), toml::to_string(node.type())));
        return false;
    }

    return true;
}

std::optional<std::u8string> pvSettingsTomlGetString(const toml::basic_value<PvSettingsTomlU8Config>& toml, const char8_t* key,
    const bool allowEmpty, const bool validateKey)
{
    if (validateKey)
    {
        if (!pvSettingsTomlValidateKey(toml, key))
        {
            return std::nullopt;
        }
    }

    if (!pvSettingsTomlValidateValue(toml, key, toml::value_t::string, allowEmpty))
    {
        return std::nullopt;
    }


    return toml.at(key).as_string();
}

std::optional<long long> pvSettingsTomlGetInteger(const toml::basic_value<PvSettingsTomlU8Config>& toml,
                                                     const char8_t* key, const bool allowEmpty, const bool validateKey)
{
    if (validateKey)
    {
        if (!pvSettingsTomlValidateKey(toml, key))
        {
            return std::nullopt;
        }
    }

    if (!pvSettingsTomlValidateValue(toml, key, toml::value_t::integer, allowEmpty))
    {
        return std::nullopt;
    }


    return toml.at(key).as_integer();
}

std::optional<bool> pvSettingsTomlGetBoolean(const toml::basic_value<PvSettingsTomlU8Config>& toml,
                                                  const char8_t* key, const bool allowEmpty, const bool validateKey)
{
    if (validateKey)
    {
        if (!pvSettingsTomlValidateKey(toml, key))
        {
            return std::nullopt;
        }
    }

    if (!pvSettingsTomlValidateValue(toml, key, toml::value_t::boolean, allowEmpty))
    {
        return std::nullopt;
    }


    return toml.at(key).as_boolean();
}

std::optional<double> pvSettingsTomlGetFp(const toml::basic_value<PvSettingsTomlU8Config>& toml, const char8_t* key,
                                             const bool allowEmpty, const bool validateKey)
{
    if (validateKey)
    {
        if (!pvSettingsTomlValidateKey(toml, key))
        {
            return std::nullopt;
        }
    }

    if (!pvSettingsTomlValidateValue(toml, key, toml::value_t::floating, allowEmpty))
    {
        return std::nullopt;
    }


    return toml.at(key).as_floating();
}

std::optional<std::reference_wrapper<const toml::basic_value<PvSettingsTomlU8Config>>> pvSettingsTomlGetNode(
    const toml::basic_value<PvSettingsTomlU8Config>& toml, const char8_t* key, const bool validateKey)
{
    if (validateKey)
    {
        if (!pvSettingsTomlValidateKey(toml, key))
        {
            return std::nullopt;
        }
    }

    if (!pvSettingsTomlValidateValue(toml, key, toml::value_t::table, false))
    {
        return std::nullopt;
    }


    return std::ref(toml.at(key));
}

bool pvSettingsTomlIsValidVersion(const toml::basic_value<PvSettingsTomlU8Config>& toml, const char8_t* versionValue)
{
    auto versionNode = pvSettingsTomlGetString(toml, u8"Version", false, true);

    if (!versionNode)
    {
        return false;
    }

    if (versionNode.value() != versionValue)
    {
        pvPfDebugPrintLine(std::format("PvSettingsToml: Version: Unsupported version: {}",
                                       pvStringFmtAnsi(versionNode.value().c_str())));

        return false;
    }

    return true;
}
