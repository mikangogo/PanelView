#pragma once

#include <format>
#include <span>
#include <string>
#include <windows.h>

typedef void* PvPfSyncHandle;

typedef void* PvPfWindowHandle;
typedef HWND PvPfWindowNativeHandle;

struct PvPfWindowRectangle
{
    long Left;
    long Right;
    long Top;
    long Bottom;

    long Width() const
    {
        return Right - Left;
    }

    long Height() const
    {
        return Bottom - Top;
    }
};

struct PvPfWindowEventOperator
{
    struct EventData
    {
        UINT EventType = WM_NULL;
        WPARAM Param1 = 0ULL;
        LPARAM Param2 = 0LL;
        LRESULT ReturnValue = 0LL;
    };

    virtual ~PvPfWindowEventOperator() = default;

    virtual bool OnReceiveMessage()
    {
        return false;
    }

    PvPfWindowRectangle GetRect() const;
    PvPfWindowRectangle GetClientRect() const;
    bool GetTopmost() const;
    bool GetMaximized() const;
    bool GetBorderless() const;

    void SetVisible(bool visible);
    void SetSize(long width, long height);
    void SetClientSize(long width, long height);
    void SetRect(const PvPfWindowRectangle& rect);
    void SetTopmost(bool isTopmost);
    void SetMaximized(bool isMaximized);
    void SetBorderless(bool isOn);


    const EventData& GetEventData() const
    {
        return _eventData;
    }

    void SetEventData(const EventData& eventData)
    {
        _eventData = eventData;
    }

    void SetEventReturnValue(LRESULT returnValue)
    {
        _eventData.ReturnValue = returnValue;
    }

    void SetNativeHandle(PvPfWindowNativeHandle nativeHandle)
    {
        _nativeHandle = nativeHandle;
    }

    PvPfWindowNativeHandle GetNativeHandle() const
    {
        return _nativeHandle;
    }

private:
    PvPfWindowNativeHandle _nativeHandle = nullptr;
    EventData _eventData;
};


typedef LRESULT PvPfEventCallbackReturnValue;


enum PvPfIoOpenControlFlag
{
    PvPfIoOpenControlFlag_Read = 0UL,
    PvPfIoOpenControlFlag_Write = 1UL << 1,
    PvPfIoOpenControlFlag_CreateNew = 1UL << 2,
};

struct PvPfFileInfo
{
    friend bool pvPfIoOpen(const char8_t* path, PvPfFileInfo& fileInfo, const unsigned int controlFlag);
    friend bool pvPfIoExpandData(PvPfFileInfo& fileInfo, const size_t desiredDataLengthBytes, bool clearExistsData);
    friend bool pvPfIoWrite(PvPfFileInfo& fileInfo);
    friend bool pvPfIoRead(PvPfFileInfo& fileInfo);
    friend void pvPfIoClose(PvPfFileInfo& fileInfo);

    PvPfFileInfo() = default;

    std::span<unsigned char> GetData() const { return std::span{_data, _dataLengthBytes}; }
    std::u8string_view GetPath() const { return std::u8string_view{_path, _pathLength}; }

    bool IsUnused() const { return _path == nullptr && _data == nullptr && _fileInstance == INVALID_HANDLE_VALUE; }

private:
    HANDLE _fileInstance = INVALID_HANDLE_VALUE;

    size_t _pathLength = 0;
    char8_t* _path = nullptr;

    size_t _dataLengthBytes = 0;
    unsigned char* _data = nullptr;
};


extern void pvPfInitialize(HINSTANCE instance, bool openConsole = false);
extern void pvPfShutdown();

extern HINSTANCE pvPfGetApplicationInstance();
extern const char8_t* pvPfGetModulePath();

extern PvPfSyncHandle pvPfCreateSyncObject();
extern void pvPfSyncLock(PvPfSyncHandle syncHandle);
extern void pvPfSyncYield(PvPfSyncHandle syncHandle);
extern void pvPfDeleteSyncObject(PvPfSyncHandle syncHandle);

extern bool pvPfIoOpen(const char8_t* path, PvPfFileInfo& fileInfo, const unsigned int controlFlag = PvPfIoOpenControlFlag_Read);
extern bool pvPfIoExpandData(PvPfFileInfo& fileInfo, const size_t desiredDataLengthBytes, bool clearExistsData = true);
extern bool pvPfIoWrite(PvPfFileInfo& fileInfo);
extern bool pvPfIoRead(PvPfFileInfo& fileInfo);
extern void pvPfIoClose(PvPfFileInfo& fileInfo);

extern size_t pvPfCccUtf8ToUtf16(char16_t* convertedBuffer, const size_t bufLength, const char8_t* source, const size_t sourceLength);
extern size_t pvPfCccMultiByteToUtf16(char16_t* convertedBuffer, const size_t bufLength, const char* source, const size_t sourceLength, const unsigned int codePage);
extern size_t pvPfCccUtf16ToUtf8(char8_t* convertedBuffer, size_t bufLength, const char16_t* source, const size_t sourceLength);
extern size_t pvPfCccMultiByteToUtf8(char8_t* convertedBuffer, size_t bufLength, const char* source, const size_t sourceLength, const unsigned int codePage);
extern unsigned int pvPfCccGetCodePageFromName(const char8_t* codePageName);

extern PvPfWindowHandle pvPfCreateWindow(const char8_t* windowTitle, PvPfWindowEventOperator& eventInfo);
extern void pvPfDeleteWindow(PvPfWindowHandle windowHandle);

extern void pvPfThrowException(const std::string& message);

#if defined(_PV_OPEN_DEBUG_CONSOLE)
extern void pvPfDebugPrintLine(const std::u8string& message);
extern void pvPfDebugPrintLine(const std::string& message);
#else
#define pvPfDebugPrintLine(...) ((void)0)
#define pvPfDebugPrintLine(...) ((void)0)
#endif

extern long long pvPfGetCurrentTime();
