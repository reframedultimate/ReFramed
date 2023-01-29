#pragma once

#include "overextension/listeners/OverextensionListener.hpp"
#include <QWidget>

class OverextensionModel;

class OverextensionView
        : public QWidget
        , public OverextensionListener
{
public:
    OverextensionView(OverextensionModel* model);
    ~OverextensionView();

private:
    void onPlayersChanged() override {}
    void onDataChanged() override {}

private:
    OverextensionModel* model_;
};
