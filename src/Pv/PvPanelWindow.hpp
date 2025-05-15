#pragma once

#include "PvEnvironmentInformationPublisher.hpp"
#include "PvInstrumentPanelDocument.hpp"
#include "PvSettings_Window.hpp"
#include "PvWindowBase.hpp"

class PvPanelWindow : public PvWindowBase
{
public:
    PvPanelWindow();
    ~PvPanelWindow() override;

    void Create(const PvInstrumentPanelDocument& document, PvEnvironmentInformationPublisher& publisher);
    void Setup(PvSettings_Window& settings);

protected:
    bool OnReceiveMessage() override;

private:
    PvWindowDataPointer _windowData;
};
