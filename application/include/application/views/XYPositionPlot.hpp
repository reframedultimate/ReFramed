#pragma once

#include "uhplot/RealtimePlot.hpp"
#include "uh/SessionListener.hpp"
#include "uh/Reference.hpp"
#include <QExplicitlySharedDataPointer>

class QActionGroup;
class QwtPlotDirectPainter;
class QwtPlotCurve;

namespace uh {
    class Session;
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
    void setSession(uh::Session* session);

protected:
    void prependContextMenuActions(QMenu* menu) override;

private:
    void onRunningSessionNewPlayerState(int player, const uh::PlayerState& state) override { (void)player; (void)state; }
    void onRunningSessionNewUniquePlayerState(int player, const uh::PlayerState& state) override;

    void onRunningGameSessionPlayerNameChanged(int player, const uh::SmallString<15>& name) override;
    void onRunningGameSessionSetNumberChanged(uh::SetNumber number) override { (void)number; }
    void onRunningGameSessionGameNumberChanged(uh::GameNumber number) override { (void)number; }
    void onRunningGameSessionFormatChanged(const uh::SetFormat& format) { (void)format; }
    void onRunningGameSessionWinnerChanged(int winner) override { (void)winner; }

    void onRunningTrainingSessionTrainingReset() override {}

private slots:
    void onDottedAction(bool enable);
    void onLinesAction(bool enable);
    void setCurveVisible(int player, bool visible);

private:
    QVector<QwtPlotCurve*> curves_;
    uh::Reference<uh::Session> session_;
    QActionGroup* curveTypeActionGroup_;
};

}
