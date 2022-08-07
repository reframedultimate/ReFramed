#pragma once

#include <QListWidget>
#include <QVector>
#include <QFileInfo>

class QFileInfo;

namespace rfapp {

class ReplayListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit ReplayListWidget(QWidget* parent=nullptr);
    ~ReplayListWidget();

    void addReplayFileName(const QFileInfo& absPathToFile);
    void removeReplayFileName(const QFileInfo& absPathToFile);
    bool itemMatchesReplayFileName(QListWidgetItem* item, const QFileInfo& absPathToFile);
    QVector<QFileInfo> selectedReplayFilePaths() const;

protected:
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QList<QListWidgetItem*> items) const override;
};

}
