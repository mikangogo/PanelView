#include "PvPanelWindow.hpp"
#include "PvGr_D3D9.hpp"
#include "PvInstrumentPanelGenerator.hpp"
#include "PvObject.hpp"
#include "PvAtomicFlag.hpp"

#include <thread>
#include <windowsx.h>


namespace
{
    enum PopUpMenuId
    {
        PopUpMenuId_SaveWindowPosition = 100,
        PopUpMenuId_Topmost = 101,
        PopUpMenuId_Borderless = 102,
    };

    struct PvPanelWindowData : PvObject<PvPanelWindowData, PvWindowDataPointer>
    {
        std::thread RenderThread;
        PvGrColor ClearColor;
        PvPanelWindow* Self = nullptr;
        const PvInstrumentPanelDocument* Document = nullptr;
        PvSettings_Window* WindowSettings = nullptr;

        PvAtomicFlag ResetFlag;
        PvAtomicFlag StopFlag;

        std::atomic_int ZoomRatio;
        std::atomic_bool IsDragging;
        std::atomic_int MousePositionX;
        std::atomic_int MousePositionY;

        HMENU PopUpMenuHandle;
    };

    int RenderProc(PvPanelWindowData& windowData, PvEnvironmentInformationPublisher& publisher)
    {
        auto context = pvGrCreateContext(*windowData.Self);
        auto panelObject = std::make_unique<PvInstrumentPanelObject>();

        auto lastTime = pvPfGetCurrentTime();
        auto lastMouseDownState = false;

        PvInstrumentPanelGenerator::DocumentToObject(context, *panelObject, *windowData.Document);

        panelObject->SubscribeEnvironmentInformation(publisher);

        for (;;)
        {
            if (windowData.StopFlag.Get())
            {
                break;
            }
            if (windowData.ResetFlag.Get())
            {
                pvGrResetContext(context, *windowData.Self);
                windowData.ResetFlag.Set(false);
            }
            auto zoom = windowData.ZoomRatio.exchange(0, std::memory_order::relaxed);
            auto isDragging = windowData.IsDragging.load(std::memory_order::relaxed);
            auto mouseX = windowData.MousePositionX.load(std::memory_order::relaxed);
            auto mouseY = windowData.MousePositionY.load(std::memory_order::relaxed);

            if (lastMouseDownState != isDragging && isDragging)
            {
                panelObject->OnMouseDown(mouseX, mouseY);
            }
            else if (lastMouseDownState != isDragging && !isDragging)
            {
                panelObject->OnMouseUp(mouseX, mouseY);
            }
            else if (isDragging)
            {
                panelObject->OnMouseDragging(mouseX, mouseY);
            }

            lastMouseDownState = isDragging;

            if (zoom != 0)
            {
                panelObject->OnMouseWheel(zoom, mouseX, mouseY);
            }

            if (!pvGrStartContext(context, windowData.ClearColor))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            const auto currentTIme = pvPfGetCurrentTime(); 
            const auto deltaTime = static_cast<double>(currentTIme - lastTime) / 1000.0;

            const double frameBufferWidth = windowData.Self->GetClientRect().Width();
            const double frameBufferHeight = windowData.Self->GetClientRect().Height();

            pvGrSetViewport(context,
                            PvGrViewPort{.X = 0.0,
                                         .Y = 0.0,
                                         .Width = frameBufferWidth,
                                         .Height = frameBufferHeight,
                                         .MinimumZ = 0.0,
                                         .MaximumZ = 1.0});
            
            panelObject->Draw(context, deltaTime);

            pvGrEndContext(context);

            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            lastTime = currentTIme;
        }

        panelObject->UnsubscribeEnvironmentInformation(publisher);

        panelObject.reset();
        pvGrDeleteContext(context);

        return 0;
    }

    void StartRender(PvPanelWindowData& windowData, const PvWindowBase& window, PvEnvironmentInformationPublisher& publisher)
    {
        windowData.RenderThread = std::thread(RenderProc, std::ref(windowData), std::ref(publisher));
    }


    void SetPopupMenuCheckState(HMENU menuHandle, PopUpMenuId menuId, bool checked)
    {
        auto checkFlags = MF_BYCOMMAND | (checked ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(menuHandle, menuId, checkFlags);
    }
}

PvPanelWindow::PvPanelWindow() : _windowData(nullptr)
{

}

PvPanelWindow::~PvPanelWindow()
{
    Delete();
}

void PvPanelWindow::Create(const PvInstrumentPanelDocument& document, PvEnvironmentInformationPublisher& publisher)
{
    PvWindowBase::Create(u8"panel");

    SetClientSize(1755, 959);


    auto windowData = new PvPanelWindowData();

    if (!GetNativeHandle())
    {
        return;
    }

    windowData->PopUpMenuHandle = CreatePopupMenu();
    AppendMenu(windowData->PopUpMenuHandle, MF_STRING, PopUpMenuId_Borderless, L"Borderless");
    AppendMenu(windowData->PopUpMenuHandle, MF_STRING, PopUpMenuId_Topmost, L"Topmost");
    AppendMenu(windowData->PopUpMenuHandle, MF_SEPARATOR, 0, nullptr);
    AppendMenu(windowData->PopUpMenuHandle, MF_STRING, PopUpMenuId_SaveWindowPosition, L"Save window position");



    windowData->ClearColor = PvGrColor::FromRgb(255, 0, 0);
    windowData->Self = this;
    windowData->Document = &document;
    StartRender(*windowData, *this, publisher);

    _windowData = windowData;
}

void PvPanelWindow::Setup(PvSettings_Window& settings)
{
    if (!PvPanelWindowData::Sanity(_windowData))
    {
        return;
    }

    auto windowData = PvPanelWindowData::HandleToPointer(_windowData);

    windowData->WindowSettings = &settings;

    windowData->ClearColor = settings.GetBackgroundColor();

    const auto& placementSettings = settings.GetWindowPlacementSettings();

    if (placementSettings)
    {
        SetRect(placementSettings.value().WindowRect);
        SetTopmost(placementSettings.value().IsTopmost);
        SetMaximized(placementSettings.value().IsMaximized);
        SetBorderless(placementSettings.value().IsBorderless);

        SetPopupMenuCheckState(windowData->PopUpMenuHandle, PopUpMenuId_Topmost, GetTopmost());
        SetPopupMenuCheckState(windowData->PopUpMenuHandle, PopUpMenuId_Borderless, GetBorderless());
    }
}

bool PvPanelWindow::OnReceiveMessage()
{
    auto& eventData = GetEventData();

    switch (eventData.EventType)
    {
        case WM_MOUSEWHEEL:
            {
                if (!PvPanelWindowData::Sanity(_windowData))
                {
                    break;
                }

                auto windowData = PvPanelWindowData::HandleToPointer(_windowData);

                auto delta = GET_WHEEL_DELTA_WPARAM(eventData.Param1);
                auto pt = POINT(GET_X_LPARAM(eventData.Param2), GET_Y_LPARAM(eventData.Param2));

                ScreenToClient(GetNativeHandle(), &pt);


                windowData->ZoomRatio.store(delta, std::memory_order::relaxed);
                windowData->MousePositionX.store(pt.x, std::memory_order::relaxed);
                windowData->MousePositionY.store(pt.y, std::memory_order::relaxed);
            }
            break;
        case WM_LBUTTONDOWN:
            {
                if (!PvPanelWindowData::Sanity(_windowData))
                {
                    break;
                }

                auto windowData = PvPanelWindowData::HandleToPointer(_windowData);

                auto pt = POINT(GET_X_LPARAM(eventData.Param2), GET_Y_LPARAM(eventData.Param2));

                ScreenToClient(GetNativeHandle(), &pt);

                SetCapture(GetNativeHandle()); 


                windowData->IsDragging.store(true, std::memory_order::relaxed);
                windowData->MousePositionX.store(pt.x, std::memory_order::relaxed);
                windowData->MousePositionY.store(pt.y, std::memory_order::relaxed);
            }
            break;
        case WM_LBUTTONUP:
            {
                if (!PvPanelWindowData::Sanity(_windowData))
                {
                    break;
                }

                auto windowData = PvPanelWindowData::HandleToPointer(_windowData);

                ReleaseCapture();

                windowData->IsDragging.store(false, std::memory_order::relaxed);
            }
            break;
        case WM_MOUSEMOVE:
            {
                if (!PvPanelWindowData::Sanity(_windowData))
                {
                    break;
                }

                auto windowData = PvPanelWindowData::HandleToPointer(_windowData);

                if (!windowData->IsDragging.load(std::memory_order::relaxed))
                {
                    break;
                }

                auto pt = POINT(GET_X_LPARAM(eventData.Param2), GET_Y_LPARAM(eventData.Param2));

                ScreenToClient(GetNativeHandle(), &pt);


                windowData->MousePositionX.store(pt.x, std::memory_order::relaxed);
                windowData->MousePositionY.store(pt.y, std::memory_order::relaxed);
            }
            break;
        case WM_RBUTTONUP:
            {
                if (!PvPanelWindowData::Sanity(_windowData))
                {
                    break;
                }

                auto windowData = PvPanelWindowData::HandleToPointer(_windowData);

                auto pt = POINT(GET_X_LPARAM(eventData.Param2), GET_Y_LPARAM(eventData.Param2));

                ClientToScreen(GetNativeHandle(), &pt);


                auto selectedId = TrackPopupMenuEx(windowData->PopUpMenuHandle, TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, GetNativeHandle(), nullptr);

                switch (selectedId)
                {
                    case PopUpMenuId_SaveWindowPosition:
                        {
                            PvSettings_Window::WindowPlacementSettings settingsToStore;
                            WINDOWPLACEMENT placement{sizeof(WINDOWPLACEMENT)};
                            GetWindowPlacement(GetNativeHandle(), &placement);
                            settingsToStore.WindowRect = PvPfWindowRectangle {placement.rcNormalPosition.left, placement.rcNormalPosition.right, placement.rcNormalPosition.top, placement.rcNormalPosition.bottom};
                            settingsToStore.IsMaximized = GetMaximized();
                            settingsToStore.IsTopmost = GetTopmost();
                            windowData->WindowSettings->Store(settingsToStore);
                        }
                        break;
                    case PopUpMenuId_Topmost:
                        {
                            SetTopmost(!GetTopmost());
                            SetPopupMenuCheckState(windowData->PopUpMenuHandle, PopUpMenuId_Topmost, GetTopmost());
                        }
                        break;
                    case PopUpMenuId_Borderless:
                        {
                            SetBorderless(!GetBorderless());
                            SetPopupMenuCheckState(windowData->PopUpMenuHandle, PopUpMenuId_Borderless, GetBorderless());
                        }
                        break;
                    default:
                        break;
                }
            }
            break;
        case WM_SIZE:
            {
                if (!PvPanelWindowData::Sanity(_windowData))
                {
                    break;
                }

                auto windowData = PvPanelWindowData::HandleToPointer(_windowData);

                {
                    windowData->ResetFlag.Set(true);
                }
            }
            break;
        case WM_NCDESTROY:
            {
                if (!PvPanelWindowData::Sanity(_windowData))
                {
                    break;
                }

                auto windowData = PvPanelWindowData::HandleToPointer(_windowData);
                if (windowData->RenderThread.joinable())
                {
                    windowData->StopFlag.Set(true);
                    windowData->RenderThread.join();
                }

                DestroyMenu(windowData->PopUpMenuHandle);

                delete windowData;
                _windowData = nullptr;
            }
            break;
    }
    return PvWindowBase::OnReceiveMessage();
}
