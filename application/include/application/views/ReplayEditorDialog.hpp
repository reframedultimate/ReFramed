#pragma once

#include "rfcommon/Reference.hpp"
#include <QDialog>

namespace rfcommon {
    class Session;
}

namespace rfapp {

class MetaDataEditModel;
class ReplayManager;

class ReplayEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ReplayEditorDialog(
            ReplayManager* replayManager,
            rfcommon::Session* session,
            const QString& currentFileNameParts,
            QWidget* parent);
    ~ReplayEditorDialog();

private slots:
    void onSaveClicked();

private:
    std::unique_ptr<MetaDataEditModel> metaDataEditModel_;
    ReplayManager* replayManager_;
    rfcommon::Reference<rfcommon::Session> session_;
    const QString currentFileNameParts_;
};

}
