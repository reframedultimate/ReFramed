#pragma once

#include "overextension/listeners/OverextensionListener.hpp"
#include <QWidget>

class OverextensionModel;
class QPlainTextEdit;

namespace Ui {
    class OverextensionView;
}

namespace rfcommon {
    class MotionLabels;
}

class OverextensionView
        : public QWidget
        , public OverextensionListener
{
    Q_OBJECT

public:
    OverextensionView(OverextensionModel* model, rfcommon::MotionLabels* labels);
    ~OverextensionView();

private slots:
    void onPOVChanged(int index);
    void onEarliestEscapeOptionChanged(int value);

private:
    void onPlayersChanged() override;
    void onDataChanged() override;
    void onCurrentFighterChanged(int fighterIdx) override;

private:
    Ui::OverextensionView* ui_;
    OverextensionModel* model_;
    rfcommon::MotionLabels* labels_;
    QPlainTextEdit* text_;
    QString lastFighterPOV_;
};
