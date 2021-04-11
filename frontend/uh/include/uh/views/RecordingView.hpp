#pragma once

#include "uh/listeners/RecordingListener.hpp"
#include <QWidget>
#include <QExplicitlySharedDataPointer>

class QTreeWidgetItem;

namespace Ui {
    class RecordingView;
}

namespace uh {

class DamagePlot;
class Recording;

class RecordingView : public QWidget
                    , public RecordingListener
{
    Q_OBJECT
public:
    explicit RecordingView(QWidget* parent=nullptr);
    ~RecordingView();

public slots:
    void setRecording(Recording* recording);

private slots:
    // This is still broken, see
    // https://www.qtcentre.org/threads/66591-QwtPlot-is-broken-(size-constraints-disregarded)
    void addDamagePlotToUI();
    void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private:
    void onActiveRecordingPlayerNameChanged(int player, const QString& name) override;
    void onActiveRecordingSetNumberChanged(int number) override;
    void onActiveRecordingGameNumberChanged(int number) override;
    void onActiveRecordingFormatChanged(const SetFormat& format) override;
    void onActiveRecordingPlayerStateAdded(int player, const PlayerState& state) override;

private:
    Ui::RecordingView* ui_;
    DamagePlot* plot_;
    QTreeWidgetItem* gameInfo_;
    QTreeWidgetItem* stageIDMappings_;
    QTreeWidgetItem* fighterIDMappings_;
    QTreeWidgetItem* baseStatusIDMappings_;
    QTreeWidgetItem* specificStatusIDMappings_;
    QVector<QTreeWidgetItem*> playerData_;
    QExplicitlySharedDataPointer<Recording> recording_;
};

}
