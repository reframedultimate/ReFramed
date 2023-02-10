#pragma once

#include "overextension/listeners/OverextensionListener.hpp"
#include <QWidget>

class OverextensionModel;
class QPlainTextEdit;

namespace Ui {
    class OverextensionView;
}

class OverextensionView
        : public QWidget
        , public OverextensionListener
{
    Q_OBJECT

public:
    OverextensionView(OverextensionModel* model);
    ~OverextensionView();

private slots:
    void onPOVChanged(int index);
    void onEarliestEscapeOptionChanged(int value);

private:
    void onPlayersChanged() override;
    void onDataChanged() override;

private:
    Ui::OverextensionView* ui_;
    OverextensionModel* model_;
    QPlainTextEdit* text_;
    QString lastFighterPOV_;
};
