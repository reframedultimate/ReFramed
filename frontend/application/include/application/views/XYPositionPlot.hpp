#pragma once

#include "application/listeners/RecordingListener.hpp"
#include "application/views/RealtimePlot.hpp"
#include <QExplicitlySharedDataPointer>

class QActionGroup;
class QwtPlotDirectPainter;
class QwtPlotCurve;

namespace uh {

class Recording;
class XYPositionPlotContextMenuActions;

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

protected:
    void prependContextMenuActions(QMenu* menu) override;

private:
    void onActiveRecordingPlayerNameChanged(int player, const QString& name) override;
    void onActiveRecordingNewUniquePlayerState(int player, const PlayerState& state) override;

    void onActiveRecordingSetNumberChanged(int number) override { (void)number; }
    void onActiveRecordingGameNumberChanged(int number) override { (void)number; }
    void onActiveRecordingFormatChanged(const SetFormat& format) { (void)format; }
    void onActiveRecordingNewPlayerState(int player, const PlayerState& state) override { (void)player; (void)state; }
    void onRecordingWinnerChanged(int winner) override { (void)winner; }

private slots:
    void onDottedAction(bool enable);
    void onLinesAction(bool enable);
    void setCurveVisible(int player, bool visible);

private:
    QVector<QwtPlotCurve*> curves_;
    QExplicitlySharedDataPointer<Recording> recording_;
    QActionGroup* curveTypeActionGroup_;
};

}
