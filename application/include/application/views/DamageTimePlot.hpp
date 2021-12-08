#pragma once

#include "rfplot/RealtimePlot.hpp"
#include "rfcommon/SessionListener.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/Vector.hpp"
#include <QExplicitlySharedDataPointer>

class QwtPlotDirectPainter;
class QwtPlotCurve;

namespace rfcommon {
    class Session;
}

namespace rfapp {

class DamageTimePlot : public rfplot::RealtimePlot
                     , public rfcommon::SessionListener
{
    Q_OBJECT
public:
    explicit DamageTimePlot(QWidget* parent=nullptr);
    ~DamageTimePlot();

public slots:
    void setSession(rfcommon::Session* sessoin);
    void clear();

private:
    void onRunningSessionNewUniquePlayerState(int player, const rfcommon::PlayerState& state) override;
    void onRunningSessionNewPlayerState(int player, const rfcommon::PlayerState& state) override { (void)player; (void)state; }

    void onRunningGameSessionPlayerNameChanged(int player, const rfcommon::SmallString<15>& name) override;
    void onRunningGameSessionSetNumberChanged(rfcommon::SetNumber number) override { (void)number; }
    void onRunningGameSessionGameNumberChanged(rfcommon::GameNumber number) override { (void)number; }
    void onRunningGameSessionFormatChanged(const rfcommon::SetFormat& format) { (void)format; }
    void onRunningGameSessionWinnerChanged(int winner) override { (void)winner; }

    void onRunningTrainingSessionTrainingReset() override {}

private:
    rfcommon::SmallVector<QwtPlotCurve*, 8> curves_;
    rfcommon::SmallVector<uint32_t, 8> prevFrames_;
    rfcommon::SmallVector<float, 8> prevDamageValues_;
    rfcommon::Reference<rfcommon::Session> session_;
    float largestTimeSeen_ = 0.0;
};

}
