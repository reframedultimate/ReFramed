#pragma once

#include "rfcommon/Reference.hpp"
#include "rfcommon/Vector.hpp"
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
            const QStringList& replayFileNames,
            QWidget* parent);
    ~ReplayEditorDialog();

private slots:
    void onSaveClicked();

private:
    std::unique_ptr<MetaDataEditModel> metaDataEditModel_;
    ReplayManager* replayManager_;
    rfcommon::SmallVector<rfcommon::Reference<rfcommon::Session>, 1> loadedSessions_;
    QStringList loadedSessionFileNames_;
};

}
