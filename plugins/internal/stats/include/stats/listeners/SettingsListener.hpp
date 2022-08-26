#pragma once

class SettingsListener
{
public:
    virtual void onSettingsStatsChanged() = 0;
    virtual void onSettingsOBSChanged() = 0;
    virtual void onSettingsWSChanged() = 0;
};
