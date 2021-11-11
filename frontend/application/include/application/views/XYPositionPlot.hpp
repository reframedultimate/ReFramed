#pragma once

#include "uhplot/RealtimePlot.hpp"
#include "uh/RecordingListener.hpp"
#include "uh/Reference.hpp"
#include <QExplicitlySharedDataPointer>

class QActionGroup;
class QwtPlotDirectPainter;
class QwtPlotCurve;

namespace uh {
    class GameSession;
}

namespace uhapp {

class XYPositionPlotContextMenuActions;

class XYPositionPlot : public uhplot::RealtimePlot
                     , public uh::SessionListener
{
    Q_OBJECT
public:
    explicit XYPositionPlot(QWidget* parent=nullptr);
    ~XYPositionPlot();

public slots:
    void clear();
    void setRecording(uh::GameSession* recording);

protected:
    void prependContextMenuActions(QMenu* menu) override;

private:
    void onRunningGameSessionPlayerNameChanged(int player, const uh::SmallString<15>& name) override;
    void onRunningGameSessionNewUniquePlayerState(int player, const uh::PlayerState& state) override;

    void onRunningGameSessionSetNumberChanged(uh::SetNumber number) override { (void)number; }
    void onRunningGameSessionGameNumberChanged(uh::GameNumber number) override { (void)number; }
    void onRunningGameSessionFormatChanged(const uh::SetFormat& format) { (void)format; }
    void onRunningGameSessionNewPlayerState(int player, const uh::PlayerState& state) override { (void)player; (void)state; }
    void onRecordingWinnerChanged(int winner) override { (void)winner; }

private slots:
    void onDottedAction(bool enable);
    void onLinesAction(bool enable);
    void setCurveVisible(int player, bool visible);

private:
    QVector<QwtPlotCurve*> curves_;
    uh::Reference<uh::GameSession> recording_;
    QActionGroup* curveTypeActionGroup_;
};

}
