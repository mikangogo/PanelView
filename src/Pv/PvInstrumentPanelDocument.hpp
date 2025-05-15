#pragma once

#include <memory>
#include <vector>

#include "PvInstrumentPanelDocumentDataNode.hpp"
#include "PvLogHandlerParser.hpp"
#include "PvPf_Windows.hpp"

class PvInstrumentPanelDocument
{
    friend class PvInstrumentPanelGenerator;

public:
    static bool LoadFromFile(const PvPfFileInfo& fileInfo, PvInstrumentPanelDocument& document);

    PvInstrumentPanelDocument();

    void Add(std::unique_ptr<PvInstrumentPanelDocumentDataNodeBase>& data);

    const PvInstrumentPanelDocumentDataNode_This& GetThis() const;
    const PvInstrumentPanelDocumentDataNode_PilotLamp& GetPilotLamp(const size_t index) const;
    const PvInstrumentPanelDocumentDataNode_Needle& GetNeedle(const size_t index) const;
    const PvInstrumentPanelDocumentDataNode_DigitalGauge
    & GetDigitalGauge(const size_t index) const;
    const PvInstrumentPanelDocumentDataNode_DigitalNumber
    & GetDigitalNumber(const size_t index) const;

    size_t GetPilotLampCount() const;
    size_t GetNeedleCount() const;
    size_t GetDigitalGaugeCount() const;
    size_t GetDigitalNumberCount() const;

    const std::u8string_view& GetDocument() const;
    const std::u8string_view& GetDocumentBody() const;
    const std::filesystem::path& GetPath() const;

protected:

private:
    std::filesystem::path _documentPath;
    std::unique_ptr<char8_t[]> _documentData;
    size_t _documentDataLength;
    std::u8string_view _documentBody;
    std::u8string_view _document;

    std::vector<std::unique_ptr<PvInstrumentPanelDocumentDataNodeBase>> _dataNodes;

    std::vector<std::reference_wrapper<PvInstrumentPanelDocumentDataNode_This>> _thisReferences;
    std::vector<std::reference_wrapper<PvInstrumentPanelDocumentDataNode_PilotLamp>> _pilotLampReferences;
    std::vector<std::reference_wrapper<PvInstrumentPanelDocumentDataNode_Needle>> _needleReferences;
    std::vector<std::reference_wrapper<PvInstrumentPanelDocumentDataNode_DigitalGauge>> _digitalGaugeReferences;
    std::vector<std::reference_wrapper<PvInstrumentPanelDocumentDataNode_DigitalNumber>> _digitalNumberReferences;
};
