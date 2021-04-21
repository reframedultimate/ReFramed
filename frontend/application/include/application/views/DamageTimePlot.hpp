#pragma once

#include "application/views/RealtimePlot.hpp"
#include "uh/RecordingListener.hpp"
#include "uh/Reference.hpp"
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
    void clear();
    void setRecording(uh::Recording* recording);

private:
    void onActiveRecordingPlayerNameChanged(int player, const std::string& name) override;
    void onActiveRecordingNewUniquePlayerState(int player, const uh::PlayerState& state) override;

    void onActiveRecordingSetNumberChanged(uh::SetNumber number) override { (void)number; }
    void onActiveRecordingGameNumberChanged(uh::GameNumber number) override { (void)number; }
    void onActiveRecordingFormatChanged(const uh::SetFormat& format) { (void)format; }
    void onActiveRecordingNewPlayerState(int player, const uh::PlayerState& state) override { (void)player; (void)state; }
    void onRecordingWinnerChanged(int winner) override { (void)winner; }

private:
    std::vector<QwtPlotCurve*> curves_;
    uh::Reference<uh::Recording> recording_;
    float largestTimeSeen_ = 0.0;
};

}
