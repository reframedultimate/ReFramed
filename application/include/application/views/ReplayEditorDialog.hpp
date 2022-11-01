#pragma once

#include "rfcommon/ReplayFileParts.hpp"
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
            const rfcommon::ReplayFileParts& currentFileNameParts,
            QWidget* parent=nullptr);
    ~ReplayEditorDialog();

private slots:
    void onSaveClicked();

    void onTournamentNameChanged(const QString& name);
    void onEventNameChanged(const QString& name);

    void onTimeStartedChanged(const QDateTime& started);
    void onSetNumberChanged(int value);
    void onSetFormatChanged(int index);
    void customFormatTextChanged(const QString& text);
    void onGameNumberChanged(int value);
    void onRoundChanged(const QString& name);

    void onPlayerSponsorChanged(int fighterIdx, const QString& name);
    void onPlayerNameChanged(int fighterIdx, const QString& name);

    void onCommentatorChanged(const QString& name);

private:
    Ui::ReplayEditorDialog* ui_;
    ReplayManager* replayManager_;
    rfcommon::Reference<rfcommon::Session> session_;
    const rfcommon::ReplayFileParts currentFileNameParts_;
};

}
