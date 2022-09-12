#pragma once

#include <QDialog>

class QStringList;

namespace Ui {
    class ExportReplayPackDialog;
}

namespace rfcommon {
    class FilePathResolver;
}

namespace rfapp {

class ExportReplayPackDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportReplayPackDialog(rfcommon::FilePathResolver* pathResolver, const QStringList& replayNames, const QStringList& replayFileNames, QWidget* parent=nullptr);
    ~ExportReplayPackDialog();

private slots:
    void onSelectionChanged();
    void onChoosePackFile();
    void onExport();

private:
    Ui::ExportReplayPackDialog* ui_;
    rfcommon::FilePathResolver* pathResolver_;
    const QStringList& replayNames_;
    const QStringList& replayFileNames_;
};

}
