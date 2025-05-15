#pragma once

#include "toml.hpp"

#include <span>

struct PvSettingsTomlU8Config
{
    using comment_type = toml::preserve_comments;

    using boolean_type = bool;
    using integer_type = std::int64_t;
    using floating_type = double;
    using string_type = std::u8string; // XXX

    template <typename T>
    using array_type = std::vector<T>;
    template <typename K, typename T>
    using table_type = std::unordered_map<K, T>;

    static toml::result<integer_type, toml::error_info>
    parse_int(const std::string& str, const toml::source_location src, const std::uint8_t base)
    {
        return toml::read_int<integer_type>(str, src, base);
    }
    static toml::result<floating_type, toml::error_info> parse_float(const std::string& str,
                                                                     const toml::source_location src, const bool is_hex)
    {
        return toml::read_float<floating_type>(str, src, is_hex);
    }
};

extern bool pvSettingsTomlLoadDocumentFromFile(const std::filesystem::path& filePath,
                                   const std::function<void(const std::span<unsigned char>&)>& setDocumentMethod);

extern bool pvSettingsTomlValidateKey(const toml::basic_value<PvSettingsTomlU8Config>& toml, const char8_t* key);

extern bool pvSettingsTomlValidateValue(const toml::basic_value<PvSettingsTomlU8Config>& toml, const char8_t* key, const toml::value_t type,
                   const bool allowEmpty = false);

extern std::optional<std::u8string> pvSettingsTomlGetString(const toml::basic_value<PvSettingsTomlU8Config>& toml, const char8_t* key,
                                       const bool allowEmpty = false, const bool validateKey = false);

extern std::optional<long long> pvSettingsTomlGetInteger(const toml::basic_value<PvSettingsTomlU8Config>& toml,
                                                         const char8_t* key, const bool allowEmpty = false, const bool validateKey = false);

std::optional<bool> pvSettingsTomlGetBoolean(const toml::basic_value<PvSettingsTomlU8Config>& toml, const char8_t* key,
                                             const bool allowEmpty = false, const bool validateKey = false);

std::optional<double> pvSettingsTomlGetFp(const toml::basic_value<PvSettingsTomlU8Config>& toml, const char8_t* key,
                                          const bool allowEmpty = false, const bool validateKey = false);

extern std::optional<std::reference_wrapper<const toml::basic_value<PvSettingsTomlU8Config>>>
pvSettingsTomlGetNode(const toml::basic_value<PvSettingsTomlU8Config>& toml, const char8_t* key, const bool validateKey = false);

extern bool pvSettingsTomlIsValidVersion(const toml::basic_value<PvSettingsTomlU8Config>& toml,
                                        const char8_t* versionValue);
