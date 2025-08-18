#include "PvInstrumentPanelDocument.hpp"

#include <regex>
#include <span>
#include <ranges>

#include "PvStringUtils.hpp"

namespace 
{
    enum PvDocumentUnicodeEncoding
    {
        PvDocumentUnicodeEncoding_InvalidData = -1,
        PvDocumentUnicodeEncoding_NotUnicode = 0,
        PvDocumentUnicodeEncoding_Utf8,
        PvDocumentUnicodeEncoding_Utf16Le,
        PvDocumentUnicodeEncoding_Utf16Be,
        PvDocumentUnicodeEncoding_Utf32Le,
        PvDocumentUnicodeEncoding_Utf32Be,
    };

    enum PvParserState
    {
        PvParseState_FirstTime = 0,
        PvParseState_SectionParsed,
        PvParseState_InvalidSectionAppeared,
    };

    enum PvDocumentVersion
    {
        PvDocumentVersion_Invalid = -1,
        PvDocumentVersion_1_00 = 0,
        PvDocumentVersion_2_00,
        PvDocumentVersion_2_01,
    };

    enum PvParseDocumentErrorReason
    {
        PvParseDocumentErrorReason_NoError = 0,
        PvParseDocumentErrorReason_InvalidSection,
    };

    struct HeaderInfo
    {
        unsigned int CodePage;
        PvDocumentVersion Version;
    };

    PvDocumentVersion DetectHeaderVersion(std::string_view versionString)
    {
        std::match_results<std::string_view::const_iterator> match;
        std::regex pattern(R"(version 1.0)", std::regex_constants::ECMAScript | std::regex_constants::icase);
        auto begin = versionString.begin();
        auto end = versionString.end();

        // Find "Version 1.0" the oldest header declaration.
        if (std::regex_search(begin, end, match, pattern))
        {
            // Version 1.0
            return PvDocumentVersion_1_00;
        }


        pattern.assign(R"(\d\.\d+)");

        PvDocumentVersion version = PvDocumentVersion_Invalid;

        if (std::regex_search(begin, end, match, pattern))
        {
            if (match.str() == "2.00")
            {
                // 2.00
                version = PvDocumentVersion_2_00;
            }
            else if (match.str() == "2.01")
            {
                // 2.01
                version = PvDocumentVersion_2_01;
            }
        }

        return version;
    }

    bool DetectCodePageFromHeader(HeaderInfo& headerInfo, const char* data, const size_t length, size_t* headerOffset = nullptr)
    {
        constexpr unsigned int InvalidCodePage = 0;

        std::string_view str(data, length);
        auto begin = str.begin();
        auto end = str.end();

        auto codePage = InvalidCodePage;
        size_t headerLength = 0;


        // Find "BveTs Instrument Panel X.YY:zzzz".
        std::regex pattern(R"(BveTs Instrument Panel \d\.\d+:.+)", std::regex_constants::ECMAScript | std::regex_constants::icase);
        std::match_results<std::string_view::const_iterator> match;

        do
        {
            if (!std::regex_search(begin, end, match, pattern))
            {
                // Find "BveTs Instrument Panel X.YY".
                // If the encoding is not specified, it shall be regarded as UTF-8.
                pattern.assign(R"(BveTs Instrument Panel \d\.\d+)", std::regex_constants::ECMAScript | std::regex_constants::icase);


                if (!std::regex_search(begin, end, match, pattern))
                {
                    // Find "Version 1.0" the oldest declaration.
                    // If the encoding is not specified, it shall be regarded as UTF-8.
                    pattern.assign(R"(version 1.0)", std::regex_constants::ECMAScript | std::regex_constants::icase);


                    if (!std::regex_search(begin, end, match, pattern))
                    {
                        // No such header description.
                        // Fail.
                        break;
                    }
                }


                headerLength += match.str().length();
                codePage = pvPfCccGetCodePageFromName(u8"utf-8");

                break;
            }

            auto matchStartPosition = match.position();

            if (matchStartPosition > 0)
            {
                // The header description is not located at the beginning of the document.
                // Fail.
                break;
            }

            auto matchLength = match.str().length();
            auto encodingDelimiterPosition = match.str().find_first_of(':');

            if (encodingDelimiterPosition == std::string::npos)
            {
                // No such the Version <-> Encoding separator.
                // Fail.
                break;
            }

            auto encodingStartPosition = encodingDelimiterPosition + 1;
            auto encodingLength = matchLength - encodingStartPosition;
            auto codeNameString = match.str().substr(encodingStartPosition, encodingLength);

            codePage = pvPfCccGetCodePageFromName(reinterpret_cast<const char8_t*>(codeNameString.c_str()));
            headerLength += matchLength;

        } while (false);


        if ((codePage != pvPfCccGetCodePageFromName(u8"utf-16")) && 
            (codePage != pvPfCccGetCodePageFromName(u8"unicodeFFFE")) &&
            !IsValidCodePage(codePage))
        {
            // Unsupported or invalid encoding.
            // Fail.
            headerInfo.CodePage = InvalidCodePage;
            return false;
        }

        
        headerInfo.Version = DetectHeaderVersion(str);
        

        if (headerOffset)
        {
            // Detect the line ending pattern.

            const auto notFound = str.length();
            auto cr = notFound;
            auto lf = notFound;

            for (std::string_view::size_type i = 0; i < str.length(); ++i)
            {
                if (str[i] == '\r')
                {
                    if (cr != notFound)
                    {
                        break;
                    }

                    cr = i;
                }
                else if (str[i] == '\n')
                {
                    if ((i - cr) == 1)
                    {
                        lf = i;
                    }
                    else if (cr == notFound)
                    {
                        lf = i;
                    }

                    break;
                }
            }

            if ((cr != notFound) && (lf == notFound))
            {
                // Only [CR]
                headerLength += sizeof(char8_t);
            }
            else if ((cr == notFound) && (lf != notFound))
            {
                // Only [LF]
                headerLength += sizeof(char8_t);
            }
            else if ((cr != notFound) && (lf != notFound))
            {
                // [CR][LF]
                headerLength += sizeof(char8_t) * 2;
            }


            *headerOffset = headerLength;
        }


        headerInfo.CodePage = codePage;

        return true;
    }

    PvDocumentUnicodeEncoding DetectBomAndOffset(const char* data, const size_t length, size_t* offsetData = nullptr)
    {
        auto result = PvDocumentUnicodeEncoding_InvalidData;
        size_t offset = 0;


        if (length >= 4)
        {
            auto ar = data;
            auto b1 = static_cast<unsigned char>(ar[0]);
            auto b2 = static_cast<unsigned char>(ar[1]);
            auto b3 = static_cast<unsigned char>(ar[2]);
            auto b4 = static_cast<unsigned char>(ar[3]);

            if ((b1 == 0xEF) && (b2 == 0xBB) && (b3 == 0xBF))
            {
                // UTF-8
                offset = 3;
                result = PvDocumentUnicodeEncoding_Utf8;
            }
            else if ((b1 == 0xFF) && (b2 == 0xFE) && (b3 == 0x00) && (b4 == 0x00))
            {
                // UTF-32 LE
                offset = 4;
                result = PvDocumentUnicodeEncoding_Utf32Le;
            }
            else if ((b1 == 0x00) && (b2 == 0x00) && (b3 == 0xFE) && (b4 == 0xFF))
            {
                // UTF-32 LE
                offset = 4;
                result = PvDocumentUnicodeEncoding_Utf32Be;
            }
            else if ((b1 == 0xFF) && (b2 == 0xFE))
            {
                // UTF-16 LE
                offset = 2;
                result = PvDocumentUnicodeEncoding_Utf16Le;
            }
            else if ((b1 == 0xFE) && (b2 == 0xFF))
            {
                // UTF-16 BE
                offset = 2;
                result = PvDocumentUnicodeEncoding_Utf16Be;
            }
            else
            {
                // Not Unicode
                result = PvDocumentUnicodeEncoding_NotUnicode;
            }
        }


        if (offsetData)
        {
            *offsetData = offset;
        }

        return result;
    }

    bool ParseHeader(const PvPfFileInfo& fileInfo, HeaderInfo& parsedHeaderInfo, std::unique_ptr<char8_t[]>& generatedDocument, size_t& generatedDocumentLength, std::u8string_view& generatedDocumentBody, const PvLogHandlerParser& logger)
    {
        auto sourceData = reinterpret_cast<const char*>(fileInfo.GetData().data());
        auto sourceDataBytes = fileInfo.GetData().size_bytes();
        size_t bomOffset = 0;
        auto unicodeEncoding = DetectBomAndOffset(sourceData, sourceDataBytes, &bomOffset);

        size_t stride = 0;
        auto codePage = 0U;

        auto sourceDataWithoutBom = sourceData + bomOffset;
        auto sourceDataWithoutBomBytes = sourceDataBytes - bomOffset;
        size_t headerOffset = 0;

        HeaderInfo headerInfo;

        switch (unicodeEncoding)
        {
        default:
            case PvDocumentUnicodeEncoding_InvalidData:
            // Broken header.
            // Fail.

            logger.PrintParseHeaderMessage(
                PvLogHandlerParser::HeaderReason_BrokenHeader,
#if defined(_PV_ARCH_X86)
                std::u8string(reinterpret_cast<const char8_t*>(sourceData), (std::min)(sourceDataBytes, 4U)));
#elif defined(_PV_ARCH_AMD64)
                std::u8string(reinterpret_cast<const char8_t*>(sourceData), (std::min)(sourceDataBytes, 4ULL)));
#endif

            return false;

        case PvDocumentUnicodeEncoding_NotUnicode:
            // BOM is not Unicode pattern.
            // Try parsing the header without re-encoding.

            logger.PrintParseHeaderMessage(PvLogHandlerParser::HeaderReason_Information, u8"BOM: Nothing or Unknown pattern. Try parsing the header without re-encoding.");

            if (!DetectCodePageFromHeader(headerInfo, sourceDataWithoutBom, sourceDataWithoutBomBytes, nullptr))
            {
                // Invalid header.
                // Fail.

                if (!headerInfo.CodePage)
                {
                    logger.PrintParseHeaderMessage(PvLogHandlerParser::HeaderReason_InvalidEncoding, u8"Failed parsing or Unsupported encoding specified.");
                }

                return false;
            }

            // Detect the encoding from specifier in header.
            codePage = headerInfo.CodePage;

            if (codePage == pvPfCccGetCodePageFromName(u8"utf-8"))
            {
                // The specifier indicates UTF-8 or empty.
                unicodeEncoding = PvDocumentUnicodeEncoding_Utf8;
                logger.PrintParseHeaderMessage(PvLogHandlerParser::HeaderReason_Information, u8"Use specified encoding: UTF-8");
            }
            else
            {
                // Other encoding is indicated.
                // Probably multi-bytes encoding.
                logger.PrintParseHeaderMessage(PvLogHandlerParser::HeaderReason_Information, std::format("Use specified encoding: CP{:d}", codePage));
            }

            stride = sizeof(char);
            break;
        case PvDocumentUnicodeEncoding_Utf8:
            // BOM is UTF-8.

            codePage = pvPfCccGetCodePageFromName(u8"utf-8");
            unicodeEncoding = PvDocumentUnicodeEncoding_Utf8;
            stride = sizeof(char8_t);

            logger.PrintParseHeaderMessage(PvLogHandlerParser::HeaderReason_Information, u8"BOM: UTF-8");
            break;
        case PvDocumentUnicodeEncoding_Utf16Le:
            // BOM is UTF-16LE.

            codePage = pvPfCccGetCodePageFromName(u8"utf-16");
            unicodeEncoding = PvDocumentUnicodeEncoding_Utf16Le;
            stride = sizeof(char16_t);

            logger.PrintParseHeaderMessage(PvLogHandlerParser::HeaderReason_Information, u8"BOM: UTF-16LE");
            break;
        case PvDocumentUnicodeEncoding_Utf16Be:
            // BOM is UTF-16BE.

            codePage = pvPfCccGetCodePageFromName(u8"unicodeFFFE");
            unicodeEncoding = PvDocumentUnicodeEncoding_Utf16Be;
            stride = sizeof(char16_t);

            logger.PrintParseHeaderMessage(PvLogHandlerParser::HeaderReason_Information, u8"BOM: UTF-16BE");
            break;
        case PvDocumentUnicodeEncoding_Utf32Le:
            // BOM is UTF-32LE.
            // Unsupported. Fail.

            logger.PrintParseHeaderMessage(PvLogHandlerParser::HeaderReason_NoSupportedEncoding, u8"BOM: UTF-32LE");
            return false;

        case PvDocumentUnicodeEncoding_Utf32Be:
            // BOM is UTF-32BE.
            // Unsupported. Fail.

            logger.PrintParseHeaderMessage(PvLogHandlerParser::HeaderReason_NoSupportedEncoding, u8"BOM: UTF-32BE");
            return false;
        }


        // ------ Encoding conversion processes ------ 

        std::unique_ptr<char8_t[]> documentData;
        size_t documentDataLength = sourceDataWithoutBomBytes;

        if (unicodeEncoding == PvDocumentUnicodeEncoding_Utf16Le)
        {
            // Convert from UTF-16LE

            documentDataLength = pvPfCccUtf16ToUtf8(nullptr, 0, reinterpret_cast<const char16_t*>(sourceDataWithoutBom), sourceDataWithoutBomBytes / stride);

            if (!documentDataLength)
            {
                // Unexpected fail.

                logger.PrintParseHeaderMessage(PvLogHandlerParser::HeaderReason_FailedEncodingConversion, u8"UTF-16LE to UTF-8");
                return false;
            }

            documentData = std::make_unique<char8_t[]>(documentDataLength);

            pvPfCccUtf16ToUtf8(documentData.get(), documentDataLength, reinterpret_cast<const char16_t*>(sourceDataWithoutBom), sourceDataWithoutBomBytes / stride);

        }
        else if (unicodeEncoding == PvDocumentUnicodeEncoding_Utf16Be)
        {
            // Convert from UTF-16BE

            auto revSourceDataWithoutBom = std::make_unique<unsigned char[]>(sourceDataWithoutBomBytes);

#if defined(_PV_ARCH_X86)
            for (auto i = 0U; i < sourceDataWithoutBomBytes; i += 2U)
#elif defined(_PV_ARCH_AMD64)
            for (auto i = 0ULL; i < sourceDataWithoutBomBytes; i += 2ULL)
#endif
            {
                if ((i + 1ULL) >= sourceDataWithoutBomBytes)
                {
                    // Weird alignment...
                    revSourceDataWithoutBom[i] = sourceDataWithoutBom[i];

                    logger.PrintParseHeaderMessage(PvLogHandlerParser::HeaderReason_Information, u8"Unexpected alignment at EoF");
                    break;
                }

                revSourceDataWithoutBom[i] = sourceDataWithoutBom[i + 1];
                revSourceDataWithoutBom[i + 1] = sourceDataWithoutBom[i];
                revSourceDataWithoutBom[i + 1] = sourceDataWithoutBom[i];
            }


            documentDataLength = pvPfCccUtf16ToUtf8(nullptr, 0, reinterpret_cast<const char16_t*>(revSourceDataWithoutBom.get()), sourceDataWithoutBomBytes / stride);

            if (!documentDataLength)
            {
                // Unexpected fail.

                logger.PrintParseHeaderMessage(PvLogHandlerParser::HeaderReason_FailedEncodingConversion, u8"UTF-16LE to UTF-8");
                return false;
            }

            documentData = std::make_unique<char8_t[]>(documentDataLength);

            pvPfCccUtf16ToUtf8(documentData.get(), documentDataLength, reinterpret_cast<const char16_t*>(revSourceDataWithoutBom.get()), sourceDataWithoutBomBytes / stride);

        }
        else if (unicodeEncoding != PvDocumentUnicodeEncoding_Utf8)
        {
            // Convert from Multi-bytes encoding.

            documentDataLength = pvPfCccMultiByteToUtf8(nullptr, 0, sourceDataWithoutBom, sourceDataWithoutBomBytes, codePage);

            if (!documentDataLength)
            {
                // Unexpected fail.

                logger.PrintParseHeaderMessage(PvLogHandlerParser::HeaderReason_FailedEncodingConversion, std::format("CP{:d} to UTF-8", codePage));
                return false;
            }

            documentData = std::make_unique<char8_t[]>(documentDataLength);

            pvPfCccMultiByteToUtf8(documentData.get(), documentDataLength, sourceDataWithoutBom, sourceDataWithoutBomBytes, codePage);
        }
        else
        {
            // No conversion.

            auto measuredLength = strnlen_s(sourceDataWithoutBom, sourceDataWithoutBomBytes);
            auto isRequiredNullTerminator = false;

            if (measuredLength == sourceDataWithoutBomBytes)
            {
                documentDataLength = measuredLength + 1;
                isRequiredNullTerminator = true;
            }
            else
            {
                documentDataLength = measuredLength;
            }

            documentData = std::make_unique<char8_t[]>(documentDataLength);
            if (isRequiredNullTerminator)
            {
                documentData.get()[documentDataLength - 1] = '\0';
            }


            strncpy_s(reinterpret_cast<char*>(documentData.get()), documentDataLength, sourceDataWithoutBom, sourceDataWithoutBomBytes);
        }

        if (!DetectCodePageFromHeader(headerInfo, reinterpret_cast<const char*>(documentData.get()), documentDataLength, &headerOffset))
        {
            // Invalid header.
            // Fail.

            if (!headerInfo.CodePage)
            {
                logger.PrintParseHeaderMessage(PvLogHandlerParser::HeaderReason_InvalidEncoding, "Failed parsing or Unsupported encoding specified.");
            }

            if (headerInfo.Version == PvDocumentVersion_Invalid)
            {
                logger.PrintParseHeaderMessage(PvLogHandlerParser::HeaderReason_InvalidVersion, "Failed parsing or Unsupported version specified.");
            }

            return false;
        }

        if (codePage != headerInfo.CodePage)
        {
            // Encoding specifier and BOM 
            // Warning.

            logger.PrintParseHeaderMessage(PvLogHandlerParser::HeaderReason_InvalidEncoding, std::format("No consistency encoding: BOM is CP{:d}, but specifier is CP{:d}. Use BOM encoding.", codePage, headerInfo.CodePage));
        }
        if (headerInfo.Version == PvDocumentVersion_Invalid)
        {
            // Unsupported version specified.
            // Fail.

            logger.PrintParseHeaderMessage(PvLogHandlerParser::HeaderReason_InvalidVersion, "Unsupported version specified.");

            return false;
        }

        generatedDocument = std::move(documentData);
        generatedDocumentLength = documentDataLength;

        auto documentBody = generatedDocument.get() + headerOffset;
        auto documentBodyLength = documentDataLength - headerOffset;

        generatedDocumentBody = std::u8string_view(documentBody, documentBodyLength);

        parsedHeaderInfo = headerInfo;

        return true;
    }

    void ParseDocument(std::u8string_view body, PvInstrumentPanelDocument& target, PvDocumentVersion documentVersion)
    {
        {
            auto crLf = 0;
            auto terminate = false;
            std::u8string_view::size_type lineStartPos = 0;
            std::u8string_view::size_type lineEndPos = 0;
            std::vector<std::u8string_view> lines;

            for (std::u8string_view::size_type i = 0; i < body.length(); ++i)
            {
                auto chr = body[i];


                if (chr == '\0')
                {
                    if (!crLf)
                    {
                        lineEndPos = i;
                    }

                    terminate = true;
                    ++crLf;
                }
                else if (chr == u8'\r')
                {
                    if (!crLf)
                    {
                        lineEndPos = i;
                    }

                    ++crLf;
                }
                else if (chr == u8'\n')
                {
                    if (!crLf)
                    {
                        lineEndPos = i;
                    }

                    ++crLf;
                }
                else if ((i + 1) == body.length())
                {
                    if (crLf > 0)
                    {
                        lineStartPos = i;
                    }

                    lineEndPos = i + 1;
                    crLf = 1;
                }
                else
                {
                    if (crLf > 0)
                    {
                        lineStartPos = i;
                    }

                    crLf = 0;
                }

                if (crLf == 1)
                {
                    auto count = lineEndPos - lineStartPos;
                    auto lineBegin = body.begin() + lineStartPos;
                    auto lineEnd = lineBegin + count;

                    if (lineBegin != lineEnd)
                    {
                        for (std::u8string_view::size_type ii = 0; ii < count; ++ii)
                        {
                            auto chr = lineBegin[ii];
                            auto commentDelimFound = false;
                            
                            switch (documentVersion)
                            {
                            case PvDocumentVersion_1_00:
                                if (chr == u8';')
                                {
                                    commentDelimFound = true;
                                }
                                break;
                            default:
                                if (chr == u8'#' || chr == u8';')
                                {
                                    commentDelimFound = true;
                                }
                                break;
                            }

                            if (commentDelimFound)
                            {
                                lineEnd = lineBegin + ii;
                                count = lineEnd - lineBegin;
                                break;
                            }
                        }
                    }

                    if (lineBegin == lineEnd)
                    {
                        continue;
                    }

                    lines.emplace_back(lineBegin, lineEnd);
                }

                if (terminate)
                {
                    break;
                }
            }


            if (lines.empty())
            {
                return;
            }

            auto thisSectionAppeared = false;
            auto parseState = PvParseState_FirstTime;
            std::unique_ptr<PvInstrumentPanelDocumentDataNodeBase> node;


            for (size_t i = 0; i < lines.size(); ++i)
            {
                auto line = lines[i];
                auto trimmedLine = pvStringTrim(line);

                if (trimmedLine.empty())
                {
                    continue;
                }


                if (trimmedLine.starts_with(u8'[') && trimmedLine.ends_with(u8']'))
                {
                    // ^\[.+\]

                    if (trimmedLine.length() < 3)
                    {
                        // Empty section
                        parseState = PvParseState_InvalidSectionAppeared;
                        continue;
                    }

                    auto sectionName = trimmedLine.substr(1, trimmedLine.length() - 2);
                    auto nodeType = PvInstrumentPanelDocumentDataNodeBase::GetNodeTypeFromString(sectionName);

                    if (nodeType ==
                        PvInstrumentPanelDocumentDataNodeBase::DataNodeType_Base)
                    {
                        // Broken
                        parseState = PvParseState_InvalidSectionAppeared;
                        continue;
                    }

                    if ((nodeType ==
                         PvInstrumentPanelDocumentDataNodeBase::DataNodeType_This) &&
                        thisSectionAppeared)
                    {
                        // Broken: Multiple [this] section
                        parseState = PvParseState_InvalidSectionAppeared;
                        continue;
                    }

                    if (parseState == PvParseState_SectionParsed)
                    {
                        target.Add(node);
                    }

                    switch (nodeType)
                    {
                        case PvInstrumentPanelDocumentDataNodeBase::DataNodeType_This:
                            node = std::make_unique<PvInstrumentPanelDocumentDataNode_This>();
                            thisSectionAppeared = true;
                            break;
                        case PvInstrumentPanelDocumentDataNodeBase::DataNodeType_PilotLamp:
                            node = std::make_unique<PvInstrumentPanelDocumentDataNode_PilotLamp>();
                            break;
                        case PvInstrumentPanelDocumentDataNodeBase::DataNodeType_Needle:
                            node = std::make_unique<PvInstrumentPanelDocumentDataNode_Needle>();
                            break;
                        case PvInstrumentPanelDocumentDataNodeBase::DataNodeType_DigitalGauge:
                            node = std::make_unique<PvInstrumentPanelDocumentDataNode_DigitalGauge>();
                            break;
                        case PvInstrumentPanelDocumentDataNodeBase::DataNodeType_DigitalNumber:
                            node = std::make_unique<PvInstrumentPanelDocumentDataNode_DigitalNumber>();
                            break;
                        default:
                            node.reset();
                    }

                    parseState = node ? PvParseState_SectionParsed : PvParseState_InvalidSectionAppeared;
                    continue;
                }

                if ((parseState == PvParseState_InvalidSectionAppeared) || (parseState == PvParseState_FirstTime))
                {
                    // Do not parse Value if Section is broken.
                    continue;
                }

                auto delimiter = trimmedLine.find_first_of(u8'=');

                if (delimiter != std::u8string_view::npos)
                {
                    // ^.+=.*

                    auto key = pvStringTrim(trimmedLine.substr(0, delimiter));

                    if (key.empty())
                    {
                        continue;
                    }

                    // https://learn.microsoft.com/ja-jp/troubleshoot/windows-client/shell-experience/file-folder-name-whitespace-characters
                    auto value = pvStringTrim(trimmedLine.substr(delimiter + 1));

                    node->ParseAndAssign(key, value);
                }

                if ((parseState == PvParseState_SectionParsed) && (i == lines.size() - 1))
                {
                    target.Add(node);
                }
            }
        }
    }
}

bool PvInstrumentPanelDocument::LoadFromFile(const PvPfFileInfo& fileInfo, PvInstrumentPanelDocument& document)
{
    if (document._documentData)
    {
        return false;
    }

    auto documentPath = std::filesystem::path(fileInfo.GetPath());

    PvLogHandlerParser parserLogger;
    parserLogger.SetDocumentPath(documentPath);


    HeaderInfo headerInfo;

    if (!ParseHeader(fileInfo, headerInfo, document._documentData, document._documentDataLength, document._documentBody, parserLogger))
    {
        // Failed parsing header.
        // Abort.
        parserLogger.PrintParseMessage(1, u8"Abort parsing.");
        return false;
    }

    document._document = std::u8string_view(document._documentData.get(), document._documentDataLength);

    auto body = document.GetDocumentBody();

    ParseDocument(body, document, headerInfo.Version);

    document._documentPath = documentPath;

    return true;
}

PvInstrumentPanelDocument::PvInstrumentPanelDocument() :
    _documentDataLength(0)
{
}


void PvInstrumentPanelDocument::Add(std::unique_ptr<PvInstrumentPanelDocumentDataNodeBase>& data)
{
    _dataNodes.push_back(std::move(data));

    auto n = _dataNodes.back().get();

    switch (n->GetNodeType())
    {
        case PvInstrumentPanelDocumentDataNodeBase::DataNodeType_This:
            _thisReferences.emplace_back(*(static_cast<PvInstrumentPanelDocumentDataNode_This*>(n)));
            break;
        case PvInstrumentPanelDocumentDataNodeBase::DataNodeType_PilotLamp:
            _pilotLampReferences.emplace_back(*(static_cast<PvInstrumentPanelDocumentDataNode_PilotLamp*>(n)));
            break;
        case PvInstrumentPanelDocumentDataNodeBase::DataNodeType_Needle:
            _needleReferences.emplace_back(*(static_cast<PvInstrumentPanelDocumentDataNode_Needle*>(n)));
            break;
        case PvInstrumentPanelDocumentDataNodeBase::DataNodeType_DigitalGauge:
            _digitalGaugeReferences.emplace_back(
                *(static_cast<PvInstrumentPanelDocumentDataNode_DigitalGauge*>(n)));
            break;
        case PvInstrumentPanelDocumentDataNodeBase::DataNodeType_DigitalNumber:
            _digitalNumberReferences.emplace_back(
                *(static_cast<PvInstrumentPanelDocumentDataNode_DigitalNumber*>(n)));
            break;
        default:
            break;
    }
}

const PvInstrumentPanelDocumentDataNode_This& PvInstrumentPanelDocument::GetThis() const
{
    return _thisReferences.at(0).get();
}

const PvInstrumentPanelDocumentDataNode_PilotLamp& PvInstrumentPanelDocument::GetPilotLamp(const size_t index) const
{
    return _pilotLampReferences.at(index).get();
}

const PvInstrumentPanelDocumentDataNode_Needle
& PvInstrumentPanelDocument::GetNeedle(const size_t index) const
{
    return _needleReferences.at(index).get();
}

const PvInstrumentPanelDocumentDataNode_DigitalGauge
& PvInstrumentPanelDocument::GetDigitalGauge(const size_t index) const
{
    return _digitalGaugeReferences.at(index).get();
}

const PvInstrumentPanelDocumentDataNode_DigitalNumber
& PvInstrumentPanelDocument::GetDigitalNumber(const size_t index) const
{
    return _digitalNumberReferences.at(index).get();
}

size_t PvInstrumentPanelDocument::GetPilotLampCount() const
{
    return _pilotLampReferences.size();
}

size_t PvInstrumentPanelDocument::GetNeedleCount() const { return _needleReferences.size(); }

size_t PvInstrumentPanelDocument::GetDigitalGaugeCount() const { return _digitalGaugeReferences.size(); }

size_t PvInstrumentPanelDocument::GetDigitalNumberCount() const { return _digitalNumberReferences.size(); }

const std::u8string_view& PvInstrumentPanelDocument::GetDocument() const
{ return _document; }

const std::u8string_view& PvInstrumentPanelDocument::GetDocumentBody() const
{ return _documentBody; }

const std::filesystem::path& PvInstrumentPanelDocument::GetPath() const
{ return _documentPath; }
