#include "PvWindowBase.hpp"

#include <format>

#include "PvPf_Windows.hpp"
#include "PvObject.hpp"

namespace
{
    struct PvWindowData : PvObject<PvWindowData, PvWindowDataPointer>
    {
        PvPfWindowHandle WindowHandle = nullptr;
    };
}

PvWindowBase::PvWindowBase()
    : _windowData(nullptr)
{
    auto windowData = new PvWindowData();

    _windowData = windowData;
}

PvWindowBase::~PvWindowBase()
{
    auto windowData = PvWindowData::HandleToPointer(_windowData);

    Delete();

    delete windowData;
}

void PvWindowBase::Create(const char8_t* windowName)
{
    auto& windowData = PvWindowData::HandleToObject(_windowData);


    if (windowData.WindowHandle)
    {
        return;
    }

    auto windowHandle = pvPfCreateWindow(windowName, *this);

    windowData.WindowHandle = windowHandle;
}

void PvWindowBase::Delete()
{
    auto& windowData = PvWindowData::HandleToObject(_windowData);

    if (!windowData.WindowHandle)
    {
        return;
    }


    pvPfDeleteWindow(windowData.WindowHandle);

    windowData.WindowHandle = nullptr;
}

bool PvWindowBase::OnReceiveMessage()
{
    if (GetEventData().EventType == WM_CLOSE)
    {
        Delete();
        return false;
    }
    return PvPfWindowEventOperator::OnReceiveMessage();
}
