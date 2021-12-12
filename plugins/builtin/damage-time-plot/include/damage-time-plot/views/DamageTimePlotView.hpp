#pragma once

#include "damage-time-plot/listeners/DamageTimePlotListener.hpp"
#include "rfplot/RealtimePlot.hpp"
#include "rfcommon/Vector.hpp"

class QwtPlotDirectPainter;
class QwtPlotCurve;
class DamageTimePlotModel;

class DamageTimePlotView : public rfplot::RealtimePlot
                         , public DamageTimePlotListener
{
public:
    explicit DamageTimePlotView(DamageTimePlotModel* model, QWidget* parent=nullptr);
    ~DamageTimePlotView();

private:
    void onDamageTimePlotSessionSet(rfcommon::Session* session) override;
    void onDamageTimePlotSessionCleared(rfcommon::Session* session) override;
    void onDamageTimePlotNameChanged(int playerIdx, const rfcommon::SmallString<15>& name) override;
    void onDamageTimePlotNewValue(int playerIdx, rfcommon::Frame frame, float damage) override;

private:
    DamageTimePlotModel* model_;
    rfcommon::SmallVector<QwtPlotCurve*, 8> curves_;
    rfcommon::SmallVector<uint32_t, 8> prevFrames_;
    rfcommon::SmallVector<float, 8> prevDamageValues_;
    float largestTimeSeen_ = 0.0;
};
