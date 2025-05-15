#pragma once
#include "PvInstrumentPanelDocument.hpp"
#include "PvInstrumentPanelObject.hpp"


class PvInstrumentPanelGenerator
{
public:
    static bool DocumentToObject(
        PvGrContextHandle grContext, 
        PvInstrumentPanelObject& object,
        const PvInstrumentPanelDocument& document);
};
