#include "PvPf_Windows.hpp"
#include "PvObject.hpp"


#include <memory>
#include <process.h>
#include <stdexcept>
#include <array>
#include <filesystem>
#include <iostream>
#include <vector>
#include <locale>

#include "PvStringUtils.hpp"

namespace
{
    constexpr size_t PathLength = 512;
    constexpr unsigned int TimerPeriod = 1;

    // ================== Sync ==================
    struct PvPfSync : PvObject<PvPfSync, PvPfSyncHandle>
    {
        CRITICAL_SECTION NativeSyncObject = {};
    };

    // ================== Window ==================
    // https://learn.microsoft.com/ja-jp/windows/win32/learnwin32/writing-the-window-procedure
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        PvPfWindowEventOperator* eventInfo = nullptr;

        if (uMsg == WM_NCCREATE)
        {
            auto cs = reinterpret_cast<CREATESTRUCT*>(lParam);
            eventInfo = static_cast<PvPfWindowEventOperator*>(cs->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(eventInfo));
            eventInfo->SetNativeHandle(hwnd);
        }
        else
        {
            eventInfo = reinterpret_cast<PvPfWindowEventOperator*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }


        if (eventInfo)
        {
            if (eventInfo->GetNativeHandle() != hwnd)
            {
                pvPfThrowException("Failed: WindowProc(): NativeWindowHandle != hwnd");
            }

            eventInfo->SetEventData({.EventType = uMsg, .Param1 = wParam, .Param2 = lParam, .ReturnValue = 0});

            if (eventInfo->OnReceiveMessage())
            {
                 return eventInfo->GetEventData().ReturnValue;
            }
        }

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    struct PvPfWindowClassData
    {
        PvPfWindowClassData(HINSTANCE appInstance, WNDPROC windowProc)
        {
            WindowClassName = GenerateWindowClassName(appInstance);

            Wc = {};
            Wc.lpfnWndProc = windowProc;
            Wc.hInstance = appInstance;
            Wc.lpszClassName = WindowClassName.c_str();

            if (!RegisterClass(&Wc))
            {
                pvPfThrowException("Failed: RegisterClass()");
            }
        }

        ~PvPfWindowClassData() { UnregisterClass(WindowClassName.c_str(), Wc.hInstance); }

        WNDCLASS Wc;
        std::wstring WindowClassName;

    private:
        static std::wstring GenerateWindowClassName(HINSTANCE applicationInstance)
        {
            return L"PanelView_" + std::to_wstring(reinterpret_cast<size_t>(applicationInstance));
        }
    };

    struct PvPfWindowData : PvObject<PvPfWindowData, PvPfWindowHandle>
    {
        PvPfWindowEventOperator* EventInfo;
    };

    // ================== Encoding ==================
    struct PvPfEncodingInfo
    {
        unsigned int CodePage;
        std::u8string Name;
    };

    // ================== Self Instance ==================
    struct PvPfInstance
    {
        HINSTANCE ApplicationInstance = nullptr;
        std::unique_ptr<char8_t[]> ModulePath;
        std::unique_ptr<PvPfWindowClassData> BaseWindowClass;
        std::vector<PvPfEncodingInfo> InstalledEncodings;
        std::vector<PvPfEncodingInfo> IanaEncodings = {
#include "PvPfEncodings_Windows.inc"
        };

        FILE* ConsoleStreamPointer = nullptr;

        LARGE_INTEGER TimerFrequency;
    };

    PvPfInstance* Instance = nullptr;

    bool IsSystemStarted()
    {
        return Instance != nullptr;
    }

    void OpenConsole()
    {
        if (AllocConsole())
        {
            freopen_s(&Instance->ConsoleStreamPointer, "CONOUT$", "w", stdout);
        }
    }

    void CloseConsole()
    {
        if (Instance->ConsoleStreamPointer != nullptr)
        {
            FreeConsole();
        }
    }

    void GrubModulePath()
    {
        std::array<wchar_t, PathLength> modulePathAndFileName;

        auto actualLength = GetModuleFileNameW(Instance->ApplicationInstance, modulePathAndFileName.data(), static_cast<DWORD>(modulePathAndFileName.size()));

        if ((actualLength == modulePathAndFileName.size()) || !actualLength)
        {
            Instance->ApplicationInstance = nullptr;
            Instance->ModulePath.reset();

            pvPfThrowException("Failed: GrubModulePath()");
        }

        {
            std::array<wchar_t, _MAX_DRIVE> drive;
            std::array<wchar_t, _MAX_DIR> directory;
            auto modulePath = std::make_unique<wchar_t[]>(PathLength);
            auto strFuncRet = 0;

            strFuncRet |= _wsplitpath_s(modulePathAndFileName.data(), drive.data(), drive.size(), directory.data(), directory.size(), nullptr, 0, nullptr, 0);
            strFuncRet |= wcscpy_s(modulePath.get(), PathLength, drive.data());
            strFuncRet |= wcscat_s(modulePath.get(), PathLength, directory.data());

            if (strFuncRet)
            {
                pvPfThrowException("Failed: GrubModulePath()");
            }


            auto u8ModulePathLength = pvPfCccUtf16ToUtf8(nullptr, 0, reinterpret_cast<const char16_t*>(modulePath.get()), PathLength);

            if (!u8ModulePathLength)
            {
                pvPfThrowException("Failed: GrubModulePath()");
            }

            auto u8ModulePath = std::make_unique<char8_t[]>(u8ModulePathLength);

            if (!pvPfCccUtf16ToUtf8(u8ModulePath.get(), u8ModulePathLength, reinterpret_cast<const char16_t*>(modulePath.get()), PathLength))
            {
                pvPfThrowException("Failed: GrubModulePath()");
            }

            Instance->ModulePath = std::move(u8ModulePath);
        }
    }

    unsigned int GetCodePageFromIanaName(const char8_t* ianaName)
    {
        unsigned int codePage = 0;

        if (!ianaName)
        {
            // err
            return 0;
        }


        auto ianaNameLength = strnlen_s(reinterpret_cast<char const*>(ianaName), MAX_PATH);
        std::u8string_view ianaNameString(ianaName, ianaNameLength);

        for (auto& encoding: Instance->IanaEncodings)
        {
            auto nameBegin = encoding.Name.begin();
            auto nameEnd = encoding.Name.end();
            auto nameIter = nameBegin;
            auto ianaNameBegin = ianaNameString.begin();
            auto ianaNameEnd = ianaNameString.end();
            auto ianaNameIter = ianaNameBegin;

            while (nameIter != nameEnd)
            {
                if (ianaNameIter == ianaNameEnd)
                {
                    break;
                }

                if (std::tolower(*nameIter) != std::tolower(*ianaNameIter))
                {
                    break;
                }


                ++ianaNameIter;
                ++nameIter;
            }

            if ((nameIter != nameEnd) || (ianaNameIter != ianaNameEnd))
            {
                continue;
            }


            codePage = encoding.CodePage;
            break;
        }

        return codePage;
    }
}

PvPfWindowRectangle PvPfWindowEventOperator::GetRect() const
{
    RECT nativeRect = {};
    PvPfWindowRectangle rect = {};

    if (GetWindowRect(GetNativeHandle(), &nativeRect))
    {
        rect.Left = nativeRect.left;
        rect.Right = nativeRect.right;
        rect.Top = nativeRect.top;
        rect.Bottom = nativeRect.bottom;
    }

    return rect;
}

PvPfWindowRectangle PvPfWindowEventOperator::GetClientRect() const
{
    RECT nativeRect = {};
    PvPfWindowRectangle rect = {};

    if (::GetClientRect(GetNativeHandle(), &nativeRect))
    {
        rect.Left = nativeRect.left;
        rect.Right = nativeRect.right;
        rect.Top = nativeRect.top;
        rect.Bottom = nativeRect.bottom;
    }

    return rect;
}

bool PvPfWindowEventOperator::GetTopmost() const
{
    auto exStyle = GetWindowLongPtr(GetNativeHandle(), GWL_EXSTYLE);
    return (exStyle & WS_EX_TOPMOST) != 0;
}

bool PvPfWindowEventOperator::GetMaximized() const
{
    return IsZoomed(GetNativeHandle()) == TRUE;
}

bool PvPfWindowEventOperator::GetBorderless() const
{
    auto style = GetWindowLongPtr(GetNativeHandle(), GWL_STYLE);
    return (style & WS_CAPTION) == 0;
}

void PvPfWindowEventOperator::SetVisible(bool visible)
{
    ShowWindow(GetNativeHandle(), visible ? SW_SHOW : SW_HIDE);
}

void PvPfWindowEventOperator::SetBorderless(bool isOn)
{
    auto style = GetWindowLongPtr(GetNativeHandle(), GWL_STYLE);

    if (isOn)
    {
        style &= ~(WS_CAPTION | WS_THICKFRAME | WS_SYSMENU);
    }
    else
    {
        style |= (WS_CAPTION | WS_THICKFRAME | WS_SYSMENU);
    }

    SetWindowLongPtr(GetNativeHandle(), GWL_STYLE, style);
    SetWindowPos(GetNativeHandle(), nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
}

void PvPfWindowEventOperator::SetSize(long width, long height)
{
    SetWindowPos(GetNativeHandle(), nullptr, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
}

void PvPfWindowEventOperator::SetClientSize(long width, long height)
{
    auto windowStyle = GetWindowLong(GetNativeHandle(), GWL_STYLE);
    RECT nativeRect = {.left = 0, .top = 0, .right = width, .bottom = height};

    AdjustWindowRect(&nativeRect, windowStyle, FALSE);

    auto newWindowWidth = nativeRect.right - nativeRect.left;
    auto newWindowHeight = nativeRect.bottom - nativeRect.top;

    SetSize(newWindowWidth, newWindowHeight);
}

void PvPfWindowEventOperator::SetRect(const PvPfWindowRectangle& rect)
{
    SetWindowPos(GetNativeHandle(), nullptr, rect.Left, rect.Top, rect.Width(), rect.Height(), SWP_NOZORDER);
}

void PvPfWindowEventOperator::SetTopmost(bool isTopmost)
{
    SetWindowPos(GetNativeHandle(),
                 isTopmost ? HWND_TOPMOST : HWND_NOTOPMOST,
                 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
    );
}


void PvPfWindowEventOperator::SetMaximized(bool isMaximized)
{
    ShowWindow(GetNativeHandle(),
               isMaximized ? SW_MAXIMIZE : SW_RESTORE
    );
}

void pvPfInitialize(const HINSTANCE instance, bool openConsole)
{
    if (IsSystemStarted() || instance == INVALID_HANDLE_VALUE || !instance)
    {
        pvPfThrowException("Failed: pvPfInitialize()");
    }

    Instance = new PvPfInstance();
    Instance->ApplicationInstance = instance;

    if (openConsole)
    {
        OpenConsole();
    }

    GrubModulePath();

    if (!QueryPerformanceFrequency(&Instance->TimerFrequency))
    {
        Instance->TimerFrequency.QuadPart = 0LL;
        pvPfDebugPrintLine(u8"pvPf: Failed setting the SystemTimerResolution.");
    }


    Instance->BaseWindowClass = std::make_unique<PvPfWindowClassData>(static_cast<HINSTANCE>(Instance->ApplicationInstance), WindowProc);

    pvPfDebugPrintLine(u8"Initialized: pvPf");
    pvPfDebugPrintLine(std::format("pvPf: HINSTANCE: {:p}, ModulePath: {}",
                                   pvStringFmtPointer(Instance->ApplicationInstance),
                                   pvStringFmtAnsi(Instance->ModulePath.get()))
    );
}

void pvPfShutdown()
{
    if (!IsSystemStarted())
    {
        pvPfThrowException("Failed: pvPfShutdown()");
    }

    pvPfDebugPrintLine(u8"Shutdown: pvPf");

    CloseConsole();

    delete Instance;
    Instance = nullptr;
}

HINSTANCE pvPfGetApplicationInstance()
{
    if (!IsSystemStarted())
    {
        return nullptr;
    }

    return Instance->ApplicationInstance;
}

const char8_t* pvPfGetModulePath()
{
    return Instance->ModulePath.get();
}

PvPfSyncHandle pvPfCreateSyncObject()
{
    auto sync = new PvPfSync();

    InitializeCriticalSection(&sync->NativeSyncObject);

    return sync;
}

void pvPfSyncLock(PvPfSyncHandle syncHandle)
{
    if (!PvPfSync::Sanity(syncHandle))
    {
        return;
    }

    auto& sync = PvPfSync::HandleToObject(syncHandle);

    EnterCriticalSection(&sync.NativeSyncObject);
}

void pvPfSyncYield(PvPfSyncHandle syncHandle)
{
    if (!PvPfSync::Sanity(syncHandle))
    {
        return;
    }

    auto& sync = PvPfSync::HandleToObject(syncHandle);

    LeaveCriticalSection(&sync.NativeSyncObject);
}

void pvPfDeleteSyncObject(PvPfSyncHandle syncHandle)
{
    if (!PvPfSync::Sanity(syncHandle))
    {
        return;
    }

    auto sync = PvPfSync::HandleToPointer(syncHandle);

    DeleteCriticalSection(&sync->NativeSyncObject);

    delete sync;
}

bool pvPfIoOpen(const char8_t* path, PvPfFileInfo& fileInfo, const unsigned int controlFlag)
{
    if (!fileInfo.IsUnused())
    {
        pvPfDebugPrintLine(std::format("pvPfIoOpen(): File already opened: fileInfo: {:p}, Path: {}",
                                       pvStringFmtPointer(&fileInfo), pvStringFmtAnsi(path)));
        return false;
    }

    pvPfDebugPrintLine(std::format("pvPfIoOpen(): File opening: fileInfo: {:p}, Path: {}",
                                   pvStringFmtPointer(&fileInfo), pvStringFmtAnsi(path)));

    std::unique_ptr<char16_t[]> fullPath;
    std::unique_ptr<char[]> u8FullPath = std::make_unique<char[]>(PathLength);

    {
        const char* u8Path = reinterpret_cast<const char*>(path);
        const char* u8ModulePath = reinterpret_cast<const char*>(Instance->ModulePath.get());

        std::array<char, _MAX_DRIVE> drive;
        std::array<char, _MAX_DIR> directory;
        std::array<char, _MAX_FNAME> fileName;
        std::array<char, _MAX_EXT> ext;

        _splitpath_s(u8Path, drive.data(), drive.size(), directory.data(), directory.size(), fileName.data(),
                     fileName.size(), ext.data(), ext.size());

        if (!strncmp(drive.data(), "\0", drive.size()))
        {
            // relative-path?? DriveLetter does not present.
            auto strFuncRet = 0;

            strFuncRet |= strcpy_s(u8FullPath.get(), PathLength, u8ModulePath);
            strFuncRet |= strcat_s(u8FullPath.get(), PathLength, directory.data());
            strFuncRet |= strcat_s(u8FullPath.get(), PathLength, fileName.data());
            strFuncRet |= strcat_s(u8FullPath.get(), PathLength, ext.data());

            if (strFuncRet)
            {
                pvPfDebugPrintLine(std::format("pvPfIoOpen(): Path normalization failed: fileInfo: {:p}, Path: {}",
                                               pvStringFmtPointer(&fileInfo), pvStringFmtAnsi(path)));
                return false;
            }

            pvPfDebugPrintLine(std::format("pvPfIoOpen(): Path normalized: fileInfo: {:p}, Path: {} -> {}",
                                           pvStringFmtPointer(&fileInfo), pvStringFmtAnsi(path),
                                           pvStringFmtAnsi(u8FullPath.get())));
        }
        else
        {
            strcpy_s(u8FullPath.get(), PathLength, u8Path);
        }

        auto desireLength =
            pvPfCccUtf8ToUtf16(nullptr, 0, reinterpret_cast<const char8_t*>(u8FullPath.get()), PathLength);

        if (desireLength <= 0)
        {
            pvPfDebugPrintLine(std::format("pvPfIoOpen(): Path encoding conversion failed: fileInfo: {:p}, Path: {}",
                                           pvStringFmtPointer(&fileInfo), pvStringFmtAnsi(path)));
            return false;
        }

        std::unique_ptr<char16_t[]> u16FullPath = std::make_unique<char16_t[]>(desireLength);

        if (!pvPfCccUtf8ToUtf16(u16FullPath.get(), desireLength, reinterpret_cast<const char8_t*>(u8FullPath.get()),
                                PathLength))
        {
            pvPfDebugPrintLine(std::format("pvPfIoOpen(): Path encoding conversion failed: fileInfo: {:p}, Path: {}",
                                           pvStringFmtPointer(&fileInfo), pvStringFmtAnsi(path)));
            return false;
        }

        fullPath = std::move(u16FullPath);
    }

    auto desiredAccessFlag = GENERIC_READ | (controlFlag & PvPfIoOpenControlFlag_Write ? GENERIC_WRITE : 0);
    auto sharingModeFlag = FILE_SHARE_READ | (controlFlag & PvPfIoOpenControlFlag_Write ? FILE_SHARE_WRITE : 0);
    auto creationFlag = (controlFlag & PvPfIoOpenControlFlag_CreateNew ? OPEN_ALWAYS : OPEN_EXISTING);
    auto fileInstance = CreateFileW(
        reinterpret_cast<LPCWSTR>(fullPath.get()),
        desiredAccessFlag,
        sharingModeFlag,
        nullptr, 
        creationFlag,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    auto failed = [](HANDLE h) -> bool
    {
        CloseHandle(h);
        return false;
    };


    switch (GetLastError())
    {
        case ERROR_SUCCESS:
            if (controlFlag & PvPfIoOpenControlFlag_CreateNew)
            {
                pvPfDebugPrintLine(
                    std::format("pvPfIoOpen(): File created: fileInfo: {:p}", pvStringFmtPointer(&fileInfo)));
            }
            break;
        case ERROR_FILE_NOT_FOUND:
            pvPfDebugPrintLine(
                std::format("pvPfIoOpen(): File not found: fileInfo: {:p}", pvStringFmtPointer(&fileInfo)));
            break;
        case ERROR_ALREADY_EXISTS:
            // Succeeded
            break;
        default:
            pvPfDebugPrintLine(
                std::format("pvPfIoOpen(): File opening failed: fileInfo: {:p}", pvStringFmtPointer(&fileInfo)));

            if (fileInstance == INVALID_HANDLE_VALUE)
            {
                break;
            }

            return failed(fileInstance);
    }


    if (fileInstance == INVALID_HANDLE_VALUE)
    {
        pvPfDebugPrintLine(
            std::format("pvPfIoOpen(): File opening failed: fileInfo: {:p}", pvStringFmtPointer(&fileInfo)));
        return false;
    }


    {
        LARGE_INTEGER fileLengthBytes;
        if (!GetFileSizeEx(fileInstance, &fileLengthBytes))
        {
            pvPfDebugPrintLine(
                std::format("pvPfIoOpen(): FileSize querying failed: fileInfo: {:p}", pvStringFmtPointer(&fileInfo)));
            return failed(fileInstance);
        }

        if (fileLengthBytes.HighPart > 0L)
        {
            // Over 4GB file.
            pvPfDebugPrintLine(
                std::format("pvPfIoOpen(): FileSize over 4GB: fileInfo: {:p}", pvStringFmtPointer(&fileInfo)));

            return failed(fileInstance);
        }

        std::u8string_view u8PathStr(reinterpret_cast<const char8_t*>(u8FullPath.get()));
        auto u8PathStrBuf = std::make_unique<char8_t[]>(u8PathStr.length());

        u8PathStr.copy(u8PathStrBuf.get(), u8PathStr.length(), 0);


        fileInfo._fileInstance = fileInstance;
        fileInfo._path = u8PathStrBuf.release();
        fileInfo._pathLength = u8PathStr.length();
        fileInfo._data = nullptr;
        fileInfo._dataLengthBytes = 0;


        if (fileLengthBytes.QuadPart > 0LL)
        {
#if defined(_PV_ARCH_X86)
            const auto dataBuf = new unsigned char[static_cast<unsigned int>(fileLengthBytes.QuadPart)];
#elif defined(_PV_ARCH_AMD64)
            const auto dataBuf = new unsigned char[fileLengthBytes.QuadPart];
#endif

            fileInfo._data = dataBuf;

#if defined(_PV_ARCH_X86)
            fileInfo._dataLengthBytes = static_cast<size_t>(fileLengthBytes.QuadPart);
#elif defined(_PV_ARCH_AMD64)
            fileInfo._dataLengthBytes = fileLengthBytes.QuadPart;
#endif
        }
    }

    return true;
}

bool pvPfIoExpandData(PvPfFileInfo& fileInfo, const size_t desiredDataLengthBytes, bool clearExistsData)
{
    if (desiredDataLengthBytes <= 0)
    {
        pvPfDebugPrintLine(std::format(
            "pvPfIoExpandData(): Invalid desiredDataLengthBytes: desiredDataLengthBytes: {:d}, fileInfo: {:p}",
            desiredDataLengthBytes, pvStringFmtPointer(&fileInfo)));
        return false;
    }

    if (fileInfo._fileInstance == INVALID_HANDLE_VALUE)
    {
        pvPfDebugPrintLine(
            std::format("pvPfIoExpandData(): Invalid file handle: fileInfo: {:p}", pvStringFmtPointer(&fileInfo)));
        return false;
    }


    auto newData = std::span{std::make_unique<unsigned char[]>(desiredDataLengthBytes).release(), desiredDataLengthBytes};

    if (fileInfo.GetData().size_bytes() == 0)
    {
        fileInfo._dataLengthBytes = newData.size_bytes();
        fileInfo._data = newData.data();

        return true;
    }

    if (!clearExistsData)
    {
        if (fileInfo.GetData().size_bytes() <= desiredDataLengthBytes)
        {
            std::ranges::copy(fileInfo.GetData(), newData.data());
        }
        else
        {
            std::ranges::copy(fileInfo.GetData().begin(),
                              fileInfo.GetData().begin() + static_cast<long long>(newData.size_bytes()),
                              newData.data());
        }
    }

    delete [] fileInfo._data;
    fileInfo._data = newData.data();
    fileInfo._dataLengthBytes = newData.size_bytes();

    return true;
}

bool pvPfIoRead(PvPfFileInfo& fileInfo)
{
    DWORD readCount = 0;


    if (fileInfo._fileInstance == INVALID_HANDLE_VALUE)
    {
        pvPfDebugPrintLine(
            std::format("pvPfIoRead(): Invalid file handle: fileInfo: {:p}", pvStringFmtPointer(&fileInfo)));
        return false;
    }


    if (!ReadFile(fileInfo._fileInstance, fileInfo.GetData().data(), static_cast<DWORD>(fileInfo.GetData().size_bytes()), &readCount, nullptr))
    {
        pvPfDebugPrintLine(
            std::format("pvPfIoRead(): File reading failed: fileInfo: {:p}", pvStringFmtPointer(&fileInfo)));
        return false;
    }

    pvPfDebugPrintLine(std::format("pvPfIoRead(): File read: fileInfo: {:p}, size: {:d}",
                                   pvStringFmtPointer(&fileInfo), readCount
    ));

    return true;
}

bool pvPfIoWrite(PvPfFileInfo& fileInfo)
{
    DWORD wroteCount = 0;


    if (fileInfo._fileInstance == INVALID_HANDLE_VALUE)
    {
        pvPfDebugPrintLine(
            std::format("pvPfIoWrite(): Invalid file handle: fileInfo: {:p}", pvStringFmtPointer(&fileInfo)));
        return false;
    }


    if (!SetFilePointerEx(fileInfo._fileInstance, LARGE_INTEGER{.QuadPart = 0ULL}, nullptr, FILE_BEGIN))
    {
        pvPfDebugPrintLine(
            std::format("pvPfIoWrite(): File writing failed: fileInfo: {:p}", pvStringFmtPointer(&fileInfo)));
        return false;
    }

    if (!WriteFile(fileInfo._fileInstance, fileInfo._data, static_cast<DWORD>(fileInfo._dataLengthBytes), &wroteCount, nullptr))
    {
        pvPfDebugPrintLine(
            std::format("pvPfIoWrite(): File writing failed: fileInfo: {:p}", pvStringFmtPointer(&fileInfo)));
        return false;
    }

    if (SetFilePointerEx(fileInfo._fileInstance, LARGE_INTEGER{.QuadPart = static_cast<LONGLONG>(fileInfo._dataLengthBytes)}, nullptr, FILE_BEGIN))
    {
        SetEndOfFile(fileInfo._fileInstance);
    }

    pvPfDebugPrintLine(std::format("pvPfIoWrite(): File wrote: fileInfo: {:p}, size: {:d}", pvStringFmtPointer(&fileInfo),
                                   wroteCount
    ));

    return true;
}

void pvPfIoClose(PvPfFileInfo& fileInfo)
{
    if (fileInfo.IsUnused())
    {
        return;
    }


    CloseHandle(fileInfo._fileInstance);

    delete[] fileInfo._data;
    delete[] fileInfo._path;


    pvPfDebugPrintLine(std::format("pvPfIoClose(): File Closed: fileInfo: {:p}",
                                   pvStringFmtPointer(&fileInfo)
    ));


    fileInfo = PvPfFileInfo();
}

size_t pvPfCccUtf8ToUtf16(char16_t* convertedBuffer, const size_t bufLength, const char8_t* source, const size_t sourceLength)
{
    return pvPfCccMultiByteToUtf16(convertedBuffer, bufLength, reinterpret_cast<const char*>(source), sourceLength, CP_UTF8);
}

size_t pvPfCccMultiByteToUtf16(char16_t* convertedBuffer, const size_t bufLength, const char* source, const size_t sourceLength, const unsigned int codePage)
{
    if (!IsValidCodePage(codePage))
    {
        // Invalid CodePage.
        return 0;
    }


    auto flags = MB_PRECOMPOSED | MB_ERR_INVALID_CHARS;

    switch (codePage)
    {
    case 50220:
    case 50221:
    case 50222:
    case 50225:
    case 50227:
    case 50229:
    case 57002:
    case 57003:
    case 57004:
    case 57005:
    case 57006:
    case 57007:
    case 57008:
    case 57009:
    case 57010:
    case 57011:
    case 65000:
    case 42:
    
        // https://learn.microsoft.com/ja-jp/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
        flags = MB_ERR_INVALID_CHARS;
        break;
    
    default: break;
    }

    if (!source || !sourceLength)
    {
        // Invalid arguments.
        return 0;
    }


    auto src = reinterpret_cast<const char*>(source);
    auto srcLengthWithoutNullChar = strnlen_s(src, sourceLength);
    auto cnvd = reinterpret_cast<wchar_t*>(convertedBuffer);


    auto wcsLength = static_cast<size_t>(MultiByteToWideChar(codePage, flags, src, static_cast<int>(srcLengthWithoutNullChar), nullptr, 0));

    if (!wcsLength)
    {
        return 0;
    }

    if (!convertedBuffer || (bufLength <= 0))
    {
        // Returns desiring length to store the buffer.
        // No transcoding.
        goto RETURNS_LENGTH;
    }


    if (wcsLength >= bufLength)
    {
        // No space to filling '\0' character on buffer. 
        // Converted Utf16 String must be Null-Terminated String.
        return 0;
    }

    if (!MultiByteToWideChar(codePage, flags, src, static_cast<int>(srcLengthWithoutNullChar), cnvd, static_cast<int>(wcsLength)))
    {
        return 0;
    }

    if (wcsLength < bufLength)
    {
        convertedBuffer[wcsLength] = '\0';
    }

RETURNS_LENGTH:
    return wcsLength + 1;       // Transcoded characters + '\0'.
}

size_t pvPfCccUtf16ToUtf8(char8_t* convertedBuffer, size_t bufLength, const char16_t* source, const size_t sourceLength)
{
    if (!source || !sourceLength)
    {
        // Invalid arguments.
        return 0;
    }


    auto flags = WC_ERR_INVALID_CHARS;
    auto src = reinterpret_cast<const wchar_t*>(source);
    auto srcLengthWithoutNullChar = wcsnlen_s(src, sourceLength);
    auto cnvd = reinterpret_cast<char*>(convertedBuffer);


    auto mbcsLength = static_cast<size_t>(WideCharToMultiByte(CP_UTF8, flags, src, static_cast<int>(srcLengthWithoutNullChar), nullptr, 0, nullptr, nullptr));

    if (!mbcsLength)
    {
        return 0;
    }

    if (!convertedBuffer || (bufLength <= 0))
    {
        // Returns desiring length to store the buffer.
        // No transcoding.
        goto RETURNS_LENGTH;
    }


    if (mbcsLength >= bufLength)
    {
        // No space to filling '\0' character on buffer. 
        // Converted Utf16 String must be Null-Terminated String.
        return 0;
    }

    if (!WideCharToMultiByte(CP_UTF8, flags, src, static_cast<int>(srcLengthWithoutNullChar), cnvd, static_cast<int>(mbcsLength), nullptr, nullptr))
    {
        return 0;
    }

    if (mbcsLength < bufLength)
    {
        convertedBuffer[mbcsLength] = '\0';
    }

RETURNS_LENGTH:
    return mbcsLength + 1;
}

size_t pvPfCccMultiByteToUtf8(char8_t* convertedBuffer, size_t bufLength, const char* source, const size_t sourceLength,
    const unsigned int codePage)
{
    std::unique_ptr<char16_t[]> convertedUtf16Buffer;
    size_t desiredUtf16BufferLength = pvPfCccMultiByteToUtf16(nullptr, 0, source, sourceLength, codePage);


    if (!desiredUtf16BufferLength)
    {
        return 0;
    }


    convertedUtf16Buffer = std::make_unique<char16_t[]>(desiredUtf16BufferLength);

    if (!pvPfCccMultiByteToUtf16(convertedUtf16Buffer.get(), desiredUtf16BufferLength, source, sourceLength, codePage))
    {
        return 0;
    }


    return pvPfCccUtf16ToUtf8(convertedBuffer, bufLength, convertedUtf16Buffer.get(), desiredUtf16BufferLength);
}

unsigned int pvPfCccGetCodePageFromName(const char8_t* codePageName)
{
    if (!IsSystemStarted())
    {
        pvPfThrowException("Failed: pvPfCccGetCodePageFromName()");
    }

    return GetCodePageFromIanaName(codePageName);
}

PvPfWindowHandle pvPfCreateWindow(const char8_t* windowTitle, PvPfWindowEventOperator& eventInfo)
{
    if (!IsSystemStarted())
    {
        return nullptr;
    }


    std::unique_ptr<char16_t[]> u16WindowTitle;
    {
        std::u8string_view u8WindowTitle(windowTitle);
        auto u16WindowTitleLength = pvPfCccUtf8ToUtf16(nullptr, 0, u8WindowTitle.data(), u8WindowTitle.length());


        if (!u16WindowTitleLength)
        {
            return nullptr;
        }

        u16WindowTitle = std::make_unique<char16_t[]>(u16WindowTitleLength);
        if (!pvPfCccUtf8ToUtf16(u16WindowTitle.get(), u16WindowTitleLength, u8WindowTitle.data(), u8WindowTitle.length()))
        {
            return nullptr;
        }
    }

    auto width  = 1024;
    auto height = 1024;
    auto windowData = new PvPfWindowData();
    windowData->EventInfo = &eventInfo;

    auto windowHandle = CreateWindow(Instance->BaseWindowClass.get()->WindowClassName.c_str(),
                                     reinterpret_cast<LPCWSTR>(u16WindowTitle.get()), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                     1024, 1024, NULL, NULL, Instance->ApplicationInstance, &eventInfo);

    if (!windowHandle || windowHandle != eventInfo.GetNativeHandle())
    {
        delete windowData;
        return nullptr;
    }

    pvPfDebugPrintLine(std::format("pvPfCreateWindow():  Width: {:d}, Height: {:d}, eventInfo: {:p}, windowHandle: {:p}", 
        width,
        height,
        pvStringFmtPointer(windowData->EventInfo),
        pvStringFmtPointer(windowData))
    );

    return windowData;
}

void pvPfDeleteWindow(PvPfWindowHandle windowHandle)
{
    auto windowData = PvPfWindowData::HandleToPointer(windowHandle);

    if (!PvPfWindowData::Sanity(windowHandle))
    {
        return;
    }

    if (windowData->EventInfo->GetNativeHandle())
    {
        DestroyWindow(windowData->EventInfo->GetNativeHandle());
    }

    pvPfDebugPrintLine(
    std::format("pvPfDeleteWindow(): windowHandle: {:p}", 
        pvStringFmtPointer(windowData))
    );

    delete windowData;
}

void pvPfThrowException(const std::string& message)
{
    auto buf = "[PV]: " + message;
    std::cout << buf << std::endl;
    throw std::runtime_error(buf);
}

#if defined(_PV_OPEN_DEBUG_CONSOLE)
void pvPfDebugPrintLine(const std::u8string& message)
{
    auto buf = u8"[PV]: " + message;
    auto u16Length = pvPfCccUtf8ToUtf16(nullptr, 0, buf.c_str(), buf.length());
    auto u16Buf = std::make_unique<char16_t[]>(u16Length);
    pvPfCccUtf8ToUtf16(u16Buf.get(), u16Length, buf.c_str(), buf.length());
    std::wcout << reinterpret_cast<LPCWSTR>(u16Buf.get()) << L"\n";
}

void pvPfDebugPrintLine(const std::string& message)
{
    auto u8 = std::u8string(reinterpret_cast<const char8_t*>(message.c_str()));
    pvPfDebugPrintLine(u8);
}
#endif

long long pvPfGetCurrentTime()
{
    if (!IsSystemStarted() || !Instance->TimerFrequency.QuadPart)
    {
        return 0;
    }


    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return (time.QuadPart * 1000) / Instance->TimerFrequency.QuadPart;
}
