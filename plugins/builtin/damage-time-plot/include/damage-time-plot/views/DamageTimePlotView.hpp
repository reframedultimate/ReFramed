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
    void onDamageTimePlotStartNew() override;
    void onDamageTimePlotDataChanged() override;
    void onDamageTimePlotNameChanged(int fighterIdx) override;

private:
    DamageTimePlotModel* model_;
    rfcommon::SmallVector<QwtPlotCurve*, 2> curves_;
};
