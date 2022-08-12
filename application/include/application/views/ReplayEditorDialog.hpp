#pragma once

#include "rfcommon/Reference.hpp"
#include <QDialog>

namespace Ui {
    class ReplayEditorDialog;
}

namespace rfcommon {
    class Session;
}

namespace rfapp {

class ReplayManager;

class ReplayEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ReplayEditorDialog(
            ReplayManager* replayManager,
            rfcommon::Session* session,
            const QString& currentFileName,
            QWidget* parent=nullptr);
    ~ReplayEditorDialog();

private slots:
    void onSaveClicked();
    void onTimeStartedChanged(const QDateTime& started);
    void onGameNumberChanged(int value);
    void onSetNumberChanged(int value);
    void onSetFormatChanged(int index);
    void customFormatTextChanged(const QString& text);

    void onPlayerNameChanged(int fighterIdx, const QString& name);

private:
    Ui::ReplayEditorDialog* ui_;
    ReplayManager* replayManager_;
    rfcommon::Reference<rfcommon::Session> session_;
    const QString currentFileName_;
};

}
