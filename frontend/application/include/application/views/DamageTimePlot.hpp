#pragma once

#include "uhplot/RealtimePlot.hpp"
#include "uh/RecordingListener.hpp"
#include "uh/Reference.hpp"
#include "uh/Vector.hpp"
#include <QExplicitlySharedDataPointer>

class QwtPlotDirectPainter;
class QwtPlotCurve;

namespace uh {
    class GameSession;
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
    void setRecording(uh::GameSession* recording);
    void clear();

private:
    void onRunningGameSessionPlayerNameChanged(int player, const uh::SmallString<15>& name) override;
    void onRunningGameSessionNewUniquePlayerState(int player, const uh::PlayerState& state) override;

    void onRunningGameSessionSetNumberChanged(uh::SetNumber number) override { (void)number; }
    void onRunningGameSessionGameNumberChanged(uh::GameNumber number) override { (void)number; }
    void onRunningGameSessionFormatChanged(const uh::SetFormat& format) { (void)format; }
    void onRunningGameSessionNewPlayerState(int player, const uh::PlayerState& state) override { (void)player; (void)state; }
    void onRecordingWinnerChanged(int winner) override { (void)winner; }

private:
    uh::SmallVector<QwtPlotCurve*, 8> curves_;
    uh::SmallVector<uint32_t, 8> prevFrames_;
    uh::SmallVector<float, 8> prevDamageValues_;
    uh::Reference<uh::GameSession> recording_;
    float largestTimeSeen_ = 0.0;
};

}
