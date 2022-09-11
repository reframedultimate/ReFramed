#pragma once

#include <QDialog>

class QStringList;
class QProgressBar;

namespace Ui {
    class ExportReplayPackDialog;
}

namespace rfcommon {
    class FilePathResolver;
}

namespace rfapp {

class ProgressDialog : public QWidget
{
    Q_OBJECT

public:
    ProgressDialog(const QString& title, const QString& text, QWidget* parent=nullptr);
    ~ProgressDialog();

public slots:
    void setPercent(int percent);

private:
    QProgressBar* bar_;
};

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
