#pragma once

#include <span>
#include <filesystem>

class PvSettingsBase
{
public:
    PvSettingsBase(const PvSettingsBase& other) = delete;
    PvSettingsBase(PvSettingsBase&& other) noexcept = delete;
    PvSettingsBase& operator=(const PvSettingsBase& other) = delete;
    PvSettingsBase& operator=(PvSettingsBase&& other) noexcept = delete;

    bool Load(const std::filesystem::path& filePath)
    {
        return OnFileLoading(filePath);
    }

protected:
    explicit PvSettingsBase() = default;

    ~PvSettingsBase() = default;
    virtual bool OnFileLoading(const std::filesystem::path& filePath) = 0;

    void SetDocument(const std::span<unsigned char>& document)
    {
        auto sizeBytes = document.size_bytes();
        auto isNullTerminated = true;

        if (document[document.size() - 1] != '\0')
        {
            sizeBytes += sizeof(document[0]);
            isNullTerminated = false;
        }

        _document = std::make_unique<char8_t[]>(sizeBytes);
        std::ranges::copy(document, _document.get());

        if (!isNullTerminated)
        {
            _document[sizeBytes - 1] = '\0';
        }
    }

    std::u8string_view GetDocumentU8() const
    {
        return std::u8string_view(_document.get());
    }

    std::string_view GetDocument() const
    {
        return std::string_view(reinterpret_cast<const char*>(_document.get()));
    }

private:
    std::unique_ptr<char8_t[]> _document = nullptr;
};
