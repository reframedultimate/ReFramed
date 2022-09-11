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

    void addReplay(const QString& appearName, const QString& fileName);
    void removeReplay(const QString& fileName);
    QString itemFileName(QListWidgetItem* item) const;
    QVector<QString> selectedReplayFileNames() const;

protected:
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QList<QListWidgetItem*> items) const override;
};

}
