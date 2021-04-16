#pragma once

#include "uh/listeners/RecordingListener.hpp"
#include "uh/views/RealtimePlot.hpp"
#include <QExplicitlySharedDataPointer>

class QwtPlotDirectPainter;
class QwtPlotCurve;

namespace uh {

class Recording;

class XYPositionPlot : public RealtimePlot
                     , public RecordingListener
{
    Q_OBJECT
public:
    explicit XYPositionPlot(QWidget* parent=nullptr);
    ~XYPositionPlot();

public slots:
    void clear();
    void setRecording(Recording* recording);
    void replotAndAutoScale();

private:
    void onActiveRecordingPlayerNameChanged(int player, const QString& name) override;
    void onActiveRecordingNewUniquePlayerState(int player, const PlayerState& state) override;

    void onActiveRecordingSetNumberChanged(int number) override { (void)number; }
    void onActiveRecordingGameNumberChanged(int number) override { (void)number; }
    void onActiveRecordingFormatChanged(const SetFormat& format) { (void)format; }
    void onActiveRecordingNewPlayerState(int player, const PlayerState& state) override { (void)player; (void)state; }
    void onRecordingWinnerChanged(int winner) override { (void)winner; }

private:
    QVector<QwtPlotCurve*> curves_;
    QExplicitlySharedDataPointer<Recording> recording_;
};

}
