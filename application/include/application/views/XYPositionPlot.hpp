#pragma once

#include "rfplot/RealtimePlot.hpp"
#include "rfcommon/SessionListener.hpp"
#include "rfcommon/Reference.hpp"
#include <QExplicitlySharedDataPointer>

class QActionGroup;
class QwtPlotDirectPainter;
class QwtPlotCurve;

namespace rfcommon {
    class Session;
}

namespace rfapp {

class XYPositionPlotContextMenuActions;

class XYPositionPlot : public rfplot::RealtimePlot
                     , public rfcommon::SessionListener
{
    Q_OBJECT
public:
    explicit XYPositionPlot(QWidget* parent=nullptr);
    ~XYPositionPlot();

public slots:
    void clear();
    void setSession(rfcommon::Session* session);

protected:
    void prependContextMenuActions(QMenu* menu) override;

private:
    void onRunningSessionNewPlayerState(int player, const rfcommon::PlayerState& state) override { (void)player; (void)state; }
    void onRunningSessionNewUniquePlayerState(int player, const rfcommon::PlayerState& state) override;

    void onRunningGameSessionPlayerNameChanged(int player, const rfcommon::SmallString<15>& name) override;
    void onRunningGameSessionSetNumberChanged(rfcommon::SetNumber number) override { (void)number; }
    void onRunningGameSessionGameNumberChanged(rfcommon::GameNumber number) override { (void)number; }
    void onRunningGameSessionFormatChanged(const rfcommon::SetFormat& format) { (void)format; }
    void onRunningGameSessionWinnerChanged(int winner) override { (void)winner; }

    void onRunningTrainingSessionTrainingReset() override {}

private slots:
    void onDottedAction(bool enable);
    void onLinesAction(bool enable);
    void setCurveVisible(int player, bool visible);

private:
    QVector<QwtPlotCurve*> curves_;
    rfcommon::Reference<rfcommon::Session> session_;
    QActionGroup* curveTypeActionGroup_;
};

}
