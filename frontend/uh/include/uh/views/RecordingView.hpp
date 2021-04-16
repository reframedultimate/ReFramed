#pragma once

#include <QWidget>

namespace Ui {
    class RecordingView;
}

namespace uh {

class DamageTimePlot;
class Recording;
class RecordingDataView;
class XYPositionPlot;

class RecordingView : public QWidget
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
    void addPlotsToUI();

private:
    Ui::RecordingView* ui_;
    DamageTimePlot* damageTimePlot_;
    XYPositionPlot* xyPositionPlot_;
    RecordingDataView* recordingDataView_;
};

}
