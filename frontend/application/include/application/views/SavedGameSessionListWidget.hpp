#pragma once

#include <QListWidget>
#include <QVector>
#include <QFileInfo>

class QFileInfo;

namespace uhapp {

class SavedGameSessionListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit SavedGameSessionListWidget(QWidget* parent=nullptr);
    ~SavedGameSessionListWidget();

    void addSavedGameSessionFileName(const QFileInfo& absPathToFile);
    void removeSavedGameSessionFileName(const QFileInfo& absPathToFile);
    bool itemMatchesSavedGameSessionFileName(QListWidgetItem* item, const QFileInfo& absPathToFile);
    QVector<QFileInfo> selectedSavedGameSessionFilePaths() const;

protected:
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QList<QListWidgetItem*> items) const override;
};

}
