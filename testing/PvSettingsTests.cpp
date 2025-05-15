#include "PvTestMain.hpp"

#include <gtest/gtest.h>

#include "PvPf_Windows.hpp"
#include "PvSettings_Root.hpp"

TEST(PvSettingsTests, PvSettingsTests_correct)
{
    ASSERT_NO_THROW(pvPfInitialize(ApplicationInstance));
    {
        PvSettings_Root settingsRoot;
        ASSERT_NO_THROW(settingsRoot.Load(u8"..\\..\\..\\testing\\data\\panelview.toml"));
    }
    ASSERT_NO_THROW(pvPfShutdown());
}
