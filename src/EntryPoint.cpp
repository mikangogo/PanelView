#if defined(_MSC_VER)
#include "Pv/PvPf_Windows.hpp"
#include "Pv/PvGr_D3D9.hpp"
#define PV_NO_INLINE __declspec(noinline)
#endif

namespace
{
    void OnLibraryLoad(void* optionalArgument)
    {
#if defined(_MSC_VER)
#if defined(_PV_OPEN_DEBUG_CONSOLE)
        pvPfInitialize(static_cast<HINSTANCE>(optionalArgument), true);
#else
        pvPfInitialize(static_cast<HINSTANCE>(optionalArgument));
#endif
#endif
    }

    void OnLibraryDispose()
    {
        pvPfShutdown();
    }
}

#if defined(_MSC_VER)
#include <windows.h>

BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD     fdwReason,
    _In_ LPVOID    lpvReserved
)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        OnLibraryLoad(hinstDLL);
        break;
    case DLL_PROCESS_DETACH:
        OnLibraryDispose();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    
    return TRUE;
}
#else
void __attribute__((constructor)) lib_entry()
{
    OnLibraryLoad(0);
}
void __attribute__((destructor)) lib_exit()
{
    OnLibraryDispose();
}
#endif