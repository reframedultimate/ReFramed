#pragma once

class OverextensionListener
{
public:
    virtual void onPlayersChanged() = 0;
    virtual void onDataChanged() = 0;
};
