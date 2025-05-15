#include "PvTestMain.hpp"

HINSTANCE ApplicationInstance;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
    int argc = __argc;
    wchar_t** argv = __wargv;

    ApplicationInstance = hInstance;

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
