#pragma once

#include <QWidget>

namespace uh {
    class GameSession;
}

namespace Ui {
    class RecordingView;
}

namespace uhapp {

class DamageTimePlot;
class RecordingDataView;
class XYPositionPlot;

class RecordingView : public QWidget
{
    Q_OBJECT
public:
    explicit RecordingView(QWidget* parent=nullptr);
    ~RecordingView();

    void showDamagePlot();

public slots:
    void setRecording(uh::GameSession* recording);
    void clear();

private slots:
    // This is still broken, see
    // https://www.qtcentre.org/threads/66591-QwtPlot-is-broken-(size-constraints-disregarded)
    void addPlotsToUI();

private:
    Ui::RecordingView* ui_;
    DamageTimePlot* damageTimePlot_;
    XYPositionPlot* xyPositionPlot_;
    RecordingDataView* recordingDataView_;
};

}
