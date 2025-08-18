#pragma once

#include "PvInstrumentPanelDefinitions.hpp"
#include "PvStringUtils.hpp"
#include <array>
#include <filesystem>
#include <numbers>

template <size_t Length>
struct PvInstrumentPanelDataNodeConstString
{
    constexpr PvInstrumentPanelDataNodeConstString(const char8_t (&string)[Length])
    {
        for (size_t i = 0; i < Length; ++i)
        {
            _string[i] = string[i];
        }

        _string[Length] = u8'\0';
    }

    constexpr char8_t operator[](size_t index) const
    {
        return _string[index];
    }

    std::array<char8_t, Length + 1> _string;
};

template <PvInstrumentPanelDataNodeConstString Key>
struct PvInstrumentPanelDocumentDataKeyValuePairBase
{
    virtual ~PvInstrumentPanelDocumentDataKeyValuePairBase() = default;

    bool TryParseFromKeyValue(std::u8string_view key, std::u8string_view value)
    {
        std::u8string myKey(Key._string.data()), otherKey(key);

        std::transform(myKey.begin(), myKey.end(), myKey.begin(),
                       [](char8_t c) { return std::tolower(c); });
        std::transform(otherKey.begin(), otherKey.end(), otherKey.begin(), 
                        [](char8_t c) { return std::tolower(c); });

        if (myKey != otherKey)
        {
            return false;
        }

        DoReset();
        return DoParseAndAssign(pvStringTrim(value));
    }

protected:
    virtual bool DoParseAndAssign(std::u8string_view value) = 0;
    virtual void DoReset() = 0;
};

template <PvInstrumentPanelDataNodeConstString Key>
struct PvInstrumentPanelDocumentDataKeyValuePair_Subject : PvInstrumentPanelDocumentDataKeyValuePairBase<Key>
{
protected:
    bool DoParseAndAssign(std::u8string_view value) override
    {
        auto found = false;
        auto index = 0;
        
        for (size_t i = 0; i < PvInstrumentPanelSubjectKeyTitle_MaxElementCount; ++i)
        {
            std::u8string subjectKey(SubjectKeys[i]), otherSubjectKey(value);

            std::transform(subjectKey.begin(), subjectKey.end(), subjectKey.begin(),
                           [](char8_t c) { return std::tolower(c); });
            std::transform(otherSubjectKey.begin(), otherSubjectKey.end(), otherSubjectKey.begin(),
                           [](char8_t c) { return std::tolower(c); });

            auto subjectKeyPos = otherSubjectKey.starts_with(subjectKey);

            if (!subjectKeyPos)
            {
                // ("Aaa".not_starts_with("Bbb")) || (len("Aaa") >= len("Aa"))
                found = false;
                continue;
            }


            // ("Aaa".starts_with("AaaBb")) || ("Aaa".starts_with("Aaa54")) || ("Aaa".starts_with("Aaa"))
            if (subjectKey == otherSubjectKey)
            {
                // ("Aaa" == "Aaa")
                found = true;
            }
            else
            {
                // ("Aaa".starts_with("AaaBb")) || ("Aaa".starts_with("Aaa54"))
                auto validation = false;

                switch (i)
                {
                    case PvInstrumentPanelSubjectKeyTitle_Kmphd:
                    case PvInstrumentPanelSubjectKeyTitle_Bcd:
                    case PvInstrumentPanelSubjectKeyTitle_Mrd:
                    case PvInstrumentPanelSubjectKeyTitle_Doorl:
                    case PvInstrumentPanelSubjectKeyTitle_Doorr:
                    case PvInstrumentPanelSubjectKeyTitle_Ats:
                        validation = true;
                        break;
                    default:
                        validation = false;
                        break;
                }

                if (!validation)
                {
                    // ("Aaa".starts_with("AaaBb"))
                    found = false;
                    continue;
                }

                // https://timsong-cpp.github.io/cppwp/n4861/basic.string#3
                std::u8string str(otherSubjectKey.substr(subjectKey.length()));
                auto begin = reinterpret_cast<const char*>(str.data());
                auto end = reinterpret_cast<const char*>(str.data() + str.length());
                auto convResult = std::from_chars(begin, end, index);

                if (convResult.ptr != end)
                {
                    found = false;
                    break;
                }

                if (index < 0)
                {
                    found = false;
                    break;
                }

                found = true;
            }

            if (found)
            {
                SubjectKey = static_cast<PvInstrumentPanelSubjectKeyTitle>(i);
                Index = index;
                break;
            }
        }

        return found;
    }

    void DoReset() override
    {
        SubjectKey =
            PvInstrumentPanelSubjectKeyTitle_Nop;
        Index = 0;
    }

public:
    PvInstrumentPanelSubjectKeyTitle SubjectKey = PvInstrumentPanelSubjectKeyTitle_Nop;
    unsigned int Index = 0;

private:
    static constexpr std::u8string_view SubjectKeys[PvInstrumentPanelSubjectKeyTitle_MaxElementCount] =
    {
        u8"nop",
        u8"true",
        u8"kmph",
        u8"kmphd",
        u8"bc",
        u8"bcd",
        u8"mr",
        u8"mrd",
        u8"sap",
        u8"bp",
        u8"er",
        u8"am",
        u8"amabs",
        u8"door",
        u8"doorl",
        u8"doorr",
        u8"csc",
        u8"ats",
        u8"power",
        u8"brake",
        u8"hour",
        u8"min",
        u8"sec",
    };
};

template <PvInstrumentPanelDataNodeConstString Key>
struct PvInstrumentPanelDocumentDataTypeKeyValuePair_Value : PvInstrumentPanelDocumentDataKeyValuePairBase<Key>
{
protected:
    bool DoParseAndAssign(std::u8string_view value) override
    {
        std::u8string str(value);

        // https://timsong-cpp.github.io/cppwp/n4861/basic.string#3
        auto begin = reinterpret_cast<const char*>(str.data());
        auto end = reinterpret_cast<const char*>(str.data() + str.length());
        double val = 0.0;
        auto convResult = std::from_chars(begin, end, val);

        if ((convResult.ptr != end) || convResult.ec != std::errc{})
        {
            DoReset();
            return false;
        }

        Value = val;
        return true;
    }

    void DoReset() override
    {
        Value = 0.0;
    }

public:
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Value(double value = 0.0) : Value(value)
    {
    }

    double Value = 0.0;
};

template <PvInstrumentPanelDataNodeConstString Key, int DataCount, bool GrowUp = false>
struct PvInstrumentPanelDocumentDataTypeKeyValuePair_ValueArray : PvInstrumentPanelDocumentDataKeyValuePairBase<Key>
{
protected:
    bool DoParseAndAssign(std::u8string_view value) override
    {
        size_t dataCount = 0;

        for (size_t i = 0; i < value.length(); ++i)
        {
            std::u8string str(value.substr(i));
            auto delimiterPosition = str.find_first_of(u8',');

            if (delimiterPosition == std::u8string::npos)
            {
                str = pvStringTrim(str);
            }
            else
            {
                str = pvStringTrim({str.begin(), str.begin() + delimiterPosition});
            }

            // https://timsong-cpp.github.io/cppwp/n4861/basic.string#3
            auto begin = reinterpret_cast<const char*>(str.data());
            auto end = reinterpret_cast<const char*>(str.data() + str.length());
            double val = 0.0;
            auto convResult = std::from_chars(begin, end, val);
            
            if (convResult.ec != std::errc{})
            {
                DoReset();
                return false;
            }

            if (convResult.ptr == end)
            {
                // Parsing 123, "123"

                if (Value.size() <= dataCount)
                {
                    Value.push_back(val);
                }
                else
                {
                    Value.at(dataCount) = val;
                }

                ++dataCount;


                if (delimiterPosition == std::u8string::npos)
                {
                    break;
                }

                i += delimiterPosition;
                continue;
            }

            if (convResult.ptr != end)
            {
                // Parsing "123", 123

                if (convResult.ec != std::errc{})
                {
                    DoReset();
                    return false;
                }

                if (static_cast<char8_t>(*convResult.ptr) == u8',')
                {
                    auto offset = convResult.ptr - reinterpret_cast<const char*>(str.data());
                    i += offset;

                    if (Value.size() <= dataCount)
                    {
                        Value.push_back(val);
                    }
                    else
                    {
                        Value.at(dataCount) = val;
                    }

                    ++dataCount;


                    continue;
                }

                DoReset();
                return false;
            }
        }


        if (dataCount < DataCount)
        {
            // Failed: Validation.

            DoReset();
            return false;
        }

        if (!GrowUp && (dataCount > DataCount))
        {
            // Failed: Validation.

            DoReset();
            return false;
        }


        return true;
    }

    void DoReset() override
    {
        Value.clear();
        Value.assign(DataCount, 0.0);
    }

public:
    std::vector<double> Value = std::vector(DataCount, 0.0);
};

template <PvInstrumentPanelDataNodeConstString Key>
struct PvInstrumentPanelDocumentDataTypeKeyValuePair_Path : PvInstrumentPanelDocumentDataKeyValuePairBase<Key>
{
protected:
    bool DoParseAndAssign(std::u8string_view value) override
    {
        if (value.empty())
        {
            return false;
        }

        Path = value;
        return true;
    }

    void DoReset() override
    {
        Path.clear();
    }

public:
    std::filesystem::path Path;
};

template <PvInstrumentPanelDataNodeConstString Key>
struct PvInstrumentPanelDocumentDataTypeKeyValuePair_Color : PvInstrumentPanelDocumentDataKeyValuePairBase<Key>
{
protected:
    bool DoParseAndAssign(std::u8string_view value) override
    {
        std::u8string str(value);

        if (str.length() < 6 || str.length() > 7)
        {
            return false;
        }

        if (str.length() == 6 && str[0] == u8'#')
        {
            return false;
        }

        if (str.length() == 7 && str[0] != u8'#')
        {
            return false;
        }

        // (u8"000000".length() == 6) && (str[0] != u8'#')
        // (u8"#000000".length() == 7) && (str[0] == u8'#')

        // https://timsong-cpp.github.io/cppwp/n4861/basic.string#3
        auto begin = reinterpret_cast<const char*>(str.data());
        auto end = reinterpret_cast<const char*>(str.data() + str.length());
        int hexColorRgb = 0;

        if (begin[0] == '#')
        {
            begin += 1;
        }

        auto convResult = std::from_chars(begin, end, hexColorRgb, 16);

        if (convResult.ptr != end)
        {
            return false;
        }
        
        B = static_cast<double>((hexColorRgb & 0xFF)) / 255.0;
        G = static_cast<double>(((hexColorRgb >> 8) & 0xFF)) / 255.0;
        R = static_cast<double>(((hexColorRgb >> 16) & 0xFF)) / 255.0;

        B = std::clamp(B, 0.0, 1.0);
        G = std::clamp(G, 0.0, 1.0);
        R = std::clamp(R, 0.0, 1.0);

        A = 1.0;

        return true;
    }

    void DoReset() override
    {
        A = 1.0;
        R = 0.0;
        G = 0.0;
        B = 0.0;
    }

public:
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Color(double a = 1.0, double r = 0.0, double g = 0.0, double b = 0.0)
        : A(a),
          R(r),
          G(g),
          B(b)
    {
    }

    double A = 1.0;
    double R = 0.0;
    double G = 0.0;
    double B = 0.0;
};

template <PvInstrumentPanelDataNodeConstString Key>
struct PvInstrumentPanelDocumentDataTypeKeyValuePair_RenderingOrder
    : PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<Key>
{
protected:
    bool DoParseAndAssign(std::u8string_view value) override
    {
        return PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<Key>::DoParseAndAssign(value);
    }
};

template <PvInstrumentPanelDataNodeConstString Key, int DataCount, bool GrowUp = false>
struct PvInstrumentPanelDocumentDataTypeKeyValuePair_Angle
    : PvInstrumentPanelDocumentDataTypeKeyValuePair_ValueArray<Key, DataCount, GrowUp>
{
protected:
    bool DoParseAndAssign(std::u8string_view value) override
    {
        if (!PvInstrumentPanelDocumentDataTypeKeyValuePair_ValueArray<Key, DataCount, GrowUp>::DoParseAndAssign(value))
        {
            return false;
        }

        return true;
    }
};

template <PvInstrumentPanelDataNodeConstString Key>
struct PvInstrumentPanelDocumentDataTypeKeyValuePair_Flag : PvInstrumentPanelDocumentDataKeyValuePairBase<Key>
{
protected:
    bool DoParseAndAssign(std::u8string_view value) override
    {
        std::u8string str(value);
        std::transform(str.begin(), str.end(), str.begin(),
                       [](char8_t c)
                       {
                           unsigned char uc = static_cast<unsigned char>(c);
                           return static_cast<char8_t>(std::tolower(uc));
                       });

        auto succeed = false;

        if (str == u8"true")
        {
            Value = true;
            succeed = true;
        }

        if (str == u8"false")
        {
            Value = false;
            succeed = true;
        }

        return succeed;
    }

    void DoReset() override
    {
        Value = false;
    }

public:
    bool Value = false;
};


struct PvInstrumentPanelDocumentDataNodeBase
{
    enum DataNodeType
    {
        DataNodeType_Base = 0,
        DataNodeType_This,
        DataNodeType_PilotLamp,
        DataNodeType_Needle,
        DataNodeType_DigitalGauge,
        DataNodeType_DigitalNumber,
        DataNodeType_MaxElementCount,
    };

    PvInstrumentPanelDocumentDataNodeBase(
        DataNodeType dataNodeType = DataNodeType_Base) :
        _dataNodeType(dataNodeType)
    {
    }

    virtual ~PvInstrumentPanelDocumentDataNodeBase() = default;

    static DataNodeType GetNodeTypeFromString(const std::u8string_view& nodeTypeString)
    {
        std::u8string str(pvStringTrim(nodeTypeString));
        std::ranges::transform(str, str.begin(), [](char8_t c) { return std::tolower(c); });

        auto ret = DataNodeType_Base;

        for (auto i = 0U; i < DataNodeTypeStrings.size(); ++i)
        {
            auto s = std::u8string(DataNodeTypeStrings[i]);
            std::ranges::transform(s, s.begin(), [](char8_t c) { return std::tolower(c); });

            if (str != s)
            {
                continue;
            }

            ret = static_cast<DataNodeType>(i);
            break;
        }

        return ret;
    }

    void ParseAndAssign(const std::u8string_view& key, const std::u8string_view& value)
    {
        DoParseAndAssign(key, value);
    }

    DataNodeType GetNodeType() const
    {
        return _dataNodeType;
    }

protected:
    virtual void DoParseAndAssign(const std::u8string_view& key, const std::u8string_view& value) = 0;

private:
    static constexpr std::array<std::u8string_view, DataNodeType_MaxElementCount>
    DataNodeTypeStrings = {u8"base", u8"This", u8"PilotLamp", u8"Needle", u8"DigitalGauge", u8"DigitalNumber"};

    DataNodeType _dataNodeType = DataNodeType_Base;
};

struct PvInstrumentPanelDocumentDataNode_This : PvInstrumentPanelDocumentDataNodeBase
{
    PvInstrumentPanelDocumentDataNode_This():
        PvInstrumentPanelDocumentDataNodeBase(DataNodeType_This)
    {
    }

protected:
    void DoParseAndAssign(const std::u8string_view& key, const std::u8string_view& value) override
    {
        Resolution.TryParseFromKeyValue(key, value);
        Left.TryParseFromKeyValue(key, value);
        Top.TryParseFromKeyValue(key, value);
        Right.TryParseFromKeyValue(key, value);
        Bottom.TryParseFromKeyValue(key, value);
        DaytimeImage.TryParseFromKeyValue(key, value);
        NighttimeImage.TryParseFromKeyValue(key, value);
        TransparentColor.TryParseFromKeyValue(key, value);
        Center.TryParseFromKeyValue(key, value);
        Origin.TryParseFromKeyValue(key, value);
        Perspective.TryParseFromKeyValue(key, value);
    }

public:
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"Resolution"> Resolution;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"Left"> Left;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"Top"> Top;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"Right"> Right;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"Bottom"> Bottom;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Path<u8"DaytimeImage"> DaytimeImage;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Path<u8"NighttimeImage"> NighttimeImage;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Color<u8"TransparentColor"> TransparentColor = PvInstrumentPanelDocumentDataTypeKeyValuePair_Color<u8"TransparentColor">(0.0, 0.0, 0.0, 0.0);
    PvInstrumentPanelDocumentDataTypeKeyValuePair_ValueArray<u8"Center", 2> Center;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_ValueArray<u8"Origin", 2> Origin;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"Perspective"> Perspective;
};

struct PvInstrumentPanelDocumentDataNode_PilotLamp
    : PvInstrumentPanelDocumentDataNodeBase
{
    PvInstrumentPanelDocumentDataNode_PilotLamp() :
        PvInstrumentPanelDocumentDataNodeBase(DataNodeType_PilotLamp)
    {
    }

protected:
    void DoParseAndAssign(const std::u8string_view& key, const std::u8string_view& value) override
    {
        Subject.TryParseFromKeyValue(key, value);
        Location.TryParseFromKeyValue(key, value);
        DaytimeImage.TryParseFromKeyValue(key, value);
        NighttimeImage.TryParseFromKeyValue(key, value);
        TransparentColor.TryParseFromKeyValue(key, value);
        Layer.TryParseFromKeyValue(key, value);
    }

public:
    PvInstrumentPanelDocumentDataKeyValuePair_Subject<u8"Subject"> Subject;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_ValueArray<u8"Location", 2> Location;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Path<u8"DaytimeImage"> DaytimeImage;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Path<u8"NighttimeImage"> NighttimeImage;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Color<u8"TransparentColor"> TransparentColor = PvInstrumentPanelDocumentDataTypeKeyValuePair_Color<u8"TransparentColor">(0.0, 0.0, 0.0, 0.0);
    PvInstrumentPanelDocumentDataTypeKeyValuePair_RenderingOrder<u8"Layer"> Layer;
};

struct PvInstrumentPanelDocumentDataNode_Needle
    : PvInstrumentPanelDocumentDataNodeBase
{
    PvInstrumentPanelDocumentDataNode_Needle() :
        PvInstrumentPanelDocumentDataNodeBase(DataNodeType_Needle)
    {
    }

protected:
    void DoParseAndAssign(const std::u8string_view& key, const std::u8string_view& value) override
    {
        Subject.TryParseFromKeyValue(key, value);
        Location.TryParseFromKeyValue(key, value);
        DaytimeImage.TryParseFromKeyValue(key, value);
        NighttimeImage.TryParseFromKeyValue(key, value);
        Color.TryParseFromKeyValue(key, value);
        TransparentColor.TryParseFromKeyValue(key, value);
        Origin.TryParseFromKeyValue(key, value);
        Layer.TryParseFromKeyValue(key, value);
        Radius.TryParseFromKeyValue(key, value);
        InitialAngle.TryParseFromKeyValue(key, value);
        LastAngle.TryParseFromKeyValue(key, value);
        Minimum.TryParseFromKeyValue(key, value);
        Maximum.TryParseFromKeyValue(key, value);
        Tilt.TryParseFromKeyValue(key, value);
        StopPin.TryParseFromKeyValue(key, value);
        NaturalFreq.TryParseFromKeyValue(key, value);
        DampingRatio.TryParseFromKeyValue(key, value);
    }

public:
    PvInstrumentPanelDocumentDataKeyValuePair_Subject<u8"Subject"> Subject;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_ValueArray<u8"Location", 2> Location;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Path<u8"DaytimeImage"> DaytimeImage;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Path<u8"NighttimeImage"> NighttimeImage;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Color<u8"Color"> Color = PvInstrumentPanelDocumentDataTypeKeyValuePair_Color<u8"Color">(1.0,1.0,1.0,1.0);
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Color<u8"TransparentColor"> TransparentColor = PvInstrumentPanelDocumentDataTypeKeyValuePair_Color<u8"TransparentColor">(0.0, 0.0, 0.0, 0.0);
    PvInstrumentPanelDocumentDataTypeKeyValuePair_ValueArray<u8"Origin", 2> Origin;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_RenderingOrder<u8"Layer"> Layer;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"Radius"> Radius;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Angle<u8"InitialAngle", 1> InitialAngle;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Angle<u8"LastAngle", 1> LastAngle;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"Minimum"> Minimum;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"Maximum"> Maximum;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Angle<u8"Tilt", 2> Tilt;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Flag<u8"StopPin"> StopPin;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"NaturalFreq"> NaturalFreq = PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"NaturalFreq">(1.0);
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"DampingRatio"> DampingRatio = PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"DampingRatio">(1.0);
};

struct PvInstrumentPanelDocumentDataNode_DigitalGauge
    : PvInstrumentPanelDocumentDataNodeBase
{
    PvInstrumentPanelDocumentDataNode_DigitalGauge() :
        PvInstrumentPanelDocumentDataNodeBase(DataNodeType_DigitalGauge)
    {
    }

protected:
    void DoParseAndAssign(const std::u8string_view& key, const std::u8string_view& value) override
    {
        Subject.TryParseFromKeyValue(key, value);
        Location.TryParseFromKeyValue(key, value);
        Color.TryParseFromKeyValue(key, value);
        Layer.TryParseFromKeyValue(key, value);
        Radius.TryParseFromKeyValue(key, value);
        InitialAngle.TryParseFromKeyValue(key, value);
        LastAngle.TryParseFromKeyValue(key, value);
        Minimum.TryParseFromKeyValue(key, value);
        Maximum.TryParseFromKeyValue(key, value);
        Tilt.TryParseFromKeyValue(key, value);
        Step.TryParseFromKeyValue(key, value);
    }

public:
    PvInstrumentPanelDocumentDataKeyValuePair_Subject<u8"Subject"> Subject;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_ValueArray<u8"Location", 2> Location;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Color<u8"Color"> Color;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_RenderingOrder<u8"Layer"> Layer;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"Radius"> Radius;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Angle<u8"InitialAngle", 1> InitialAngle;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Angle<u8"LastAngle", 1> LastAngle;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"Minimum"> Minimum;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"Maximum"> Maximum;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Angle<u8"Tilt", 2> Tilt;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"Step"> Step;
};

struct PvInstrumentPanelDocumentDataNode_DigitalNumber
    : PvInstrumentPanelDocumentDataNodeBase
{
    PvInstrumentPanelDocumentDataNode_DigitalNumber() :
        PvInstrumentPanelDocumentDataNodeBase(DataNodeType_DigitalNumber)
    {
    }

protected:
    void DoParseAndAssign(const std::u8string_view& key, const std::u8string_view& value) override
    {
        Subject.TryParseFromKeyValue(key, value);
        Location.TryParseFromKeyValue(key, value);
        DaytimeImage.TryParseFromKeyValue(key, value);
        NighttimeImage.TryParseFromKeyValue(key, value);
        TransparentColor.TryParseFromKeyValue(key, value);
        Layer.TryParseFromKeyValue(key, value);
        Interval.TryParseFromKeyValue(key, value);
    }

public:
    PvInstrumentPanelDocumentDataKeyValuePair_Subject<u8"Subject"> Subject;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_ValueArray<u8"Location", 2> Location;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Path<u8"DaytimeImage"> DaytimeImage;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Path<u8"NighttimeImage"> NighttimeImage;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Color<u8"TransparentColor"> TransparentColor = PvInstrumentPanelDocumentDataTypeKeyValuePair_Color<u8"TransparentColor">(0.0, 0.0, 0.0, 0.0);
    PvInstrumentPanelDocumentDataTypeKeyValuePair_RenderingOrder<u8"Layer"> Layer;
    PvInstrumentPanelDocumentDataTypeKeyValuePair_Value<u8"Interval"> Interval;
};
