#pragma once

#include "uhplot/RealtimePlot.hpp"
#include "uh/RecordingListener.hpp"
#include "uh/Reference.hpp"
#include "uh/Vector.hpp"
#include <QExplicitlySharedDataPointer>

class QwtPlotDirectPainter;
class QwtPlotCurve;

namespace uh {
    class Recording;
}

namespace uhapp {

class DamageTimePlot : public RealtimePlot
                     , public uh::RecordingListener
{
    Q_OBJECT
public:
    explicit DamageTimePlot(QWidget* parent=nullptr);
    ~DamageTimePlot();

public slots:
    void setRecording(uh::Recording* recording);
    void clear();

private:
    void onActiveRecordingPlayerNameChanged(int player, const uh::SmallString<15>& name) override;
    void onActiveRecordingNewUniquePlayerState(int player, const uh::PlayerState& state) override;

    void onActiveRecordingSetNumberChanged(uh::SetNumber number) override { (void)number; }
    void onActiveRecordingGameNumberChanged(uh::GameNumber number) override { (void)number; }
    void onActiveRecordingFormatChanged(const uh::SetFormat& format) { (void)format; }
    void onActiveRecordingNewPlayerState(int player, const uh::PlayerState& state) override { (void)player; (void)state; }
    void onRecordingWinnerChanged(int winner) override { (void)winner; }

private:
    uh::SmallVector<QwtPlotCurve*, 8> curves_;
    uh::SmallVector<uint32_t, 8> prevFrames_;
    uh::SmallVector<float, 8> prevDamageValues_;
    uh::Reference<uh::Recording> recording_;
    float largestTimeSeen_ = 0.0;
};

}
