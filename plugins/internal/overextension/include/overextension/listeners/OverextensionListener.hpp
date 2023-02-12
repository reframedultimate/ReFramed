#pragma once

class OverextensionListener
{
public:
    virtual void onPlayersChanged() = 0;
    virtual void onDataChanged() = 0;
    virtual void onCurrentFighterChanged(int fighterIdx) = 0;
};
