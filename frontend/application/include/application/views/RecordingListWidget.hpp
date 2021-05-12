#pragma once

#include <QListWidget>
#include <QVector>
#include <QFileInfo>

class QFileInfo;

namespace uhapp {

class RecordingListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit RecordingListWidget(QWidget* parent=nullptr);
    ~RecordingListWidget();

    void addRecordingFileName(const QFileInfo& absPathToFile);
    void removeRecordingFileName(const QFileInfo& absPathToFile);
    bool itemMatchesRecordingFileName(QListWidgetItem* item, const QFileInfo& absPathToFile);
    QVector<QFileInfo> selectedRecordingFilePaths() const;

protected:
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QList<QListWidgetItem*> items) const override;
};

}
