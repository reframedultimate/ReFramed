#pragma once

#include "damage-time-plot/listeners/DamageTimePlotListener.hpp"
#include "rfcommon/Vector.hpp"
#include <QWidget>

class DamageTimePlotModel;
class QListView;
class QwtPlotCurve;

namespace rfplot {
    class RealtimePlot;
}

class DamageTimePlotView : public QWidget
                         , public DamageTimePlotListener
{
public:
    explicit DamageTimePlotView(DamageTimePlotModel* model, QWidget* parent=nullptr);
    ~DamageTimePlotView();

private:
    void clearCurves();
    void buildCurves();

private:
    void onDataSetChanged() override;
    void onDataChanged() override;
    void onNamesChanged() override;

private:
    DamageTimePlotModel* model_;
    rfplot::RealtimePlot* plot_;
    QListView* sessionsList_;
    rfcommon::SmallVector<QwtPlotCurve*, 2> curves_;
};
