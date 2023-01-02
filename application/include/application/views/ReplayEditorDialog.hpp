#pragma once

#include "application/models/ConfigAccessor.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/Vector.hpp"
#include <QDialog>

namespace rfcommon {
    class Session;
}

namespace rfapp {

class MetadataEditModel;
class MetadataEditWidget;
class PlayerDetails;
class ReplayManager;

class ReplayEditorDialog
        : public QDialog
        , public ConfigAccessor
{
    Q_OBJECT
public:
    explicit ReplayEditorDialog(
            Config* config,
            ReplayManager* replayManager,
            PlayerDetails* playerDetails,
            const QStringList& replayFileNames,
            QWidget* parent);
    ~ReplayEditorDialog();

private slots:
    void onSaveClicked();

private:
    std::unique_ptr<MetadataEditModel> metadataEditModel_;
    ReplayManager* replayManager_;
    rfcommon::SmallVector<rfcommon::Reference<rfcommon::Session>, 1> loadedSessions_;
    QStringList loadedSessionFileNames_;
    QVector<MetadataEditWidget*> metadataEditWidgets_;
};

}
