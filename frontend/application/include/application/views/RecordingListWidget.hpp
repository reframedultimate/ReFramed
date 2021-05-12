#pragma once

#include <QListWidget>

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

protected:
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QList<QListWidgetItem*> items) const override;
};

}
