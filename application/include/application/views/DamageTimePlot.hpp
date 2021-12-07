#pragma once

#include "uhplot/RealtimePlot.hpp"
#include "uh/SessionListener.hpp"
#include "uh/Reference.hpp"
#include "uh/Vector.hpp"
#include <QExplicitlySharedDataPointer>

class QwtPlotDirectPainter;
class QwtPlotCurve;

namespace uh {
    class Session;
}

namespace uhapp {

class DamageTimePlot : public uhplot::RealtimePlot
                     , public uh::SessionListener
{
    Q_OBJECT
public:
    explicit DamageTimePlot(QWidget* parent=nullptr);
    ~DamageTimePlot();

public slots:
    void setSession(uh::Session* sessoin);
    void clear();

private:
    void onRunningSessionNewUniquePlayerState(int player, const uh::PlayerState& state) override;
    void onRunningSessionNewPlayerState(int player, const uh::PlayerState& state) override { (void)player; (void)state; }

    void onRunningGameSessionPlayerNameChanged(int player, const uh::SmallString<15>& name) override;
    void onRunningGameSessionSetNumberChanged(uh::SetNumber number) override { (void)number; }
    void onRunningGameSessionGameNumberChanged(uh::GameNumber number) override { (void)number; }
    void onRunningGameSessionFormatChanged(const uh::SetFormat& format) { (void)format; }
    void onRunningGameSessionWinnerChanged(int winner) override { (void)winner; }

    void onRunningTrainingSessionTrainingReset() override {}

private:
    uh::SmallVector<QwtPlotCurve*, 8> curves_;
    uh::SmallVector<uint32_t, 8> prevFrames_;
    uh::SmallVector<float, 8> prevDamageValues_;
    uh::Reference<uh::Session> session_;
    float largestTimeSeen_ = 0.0;
};

}
