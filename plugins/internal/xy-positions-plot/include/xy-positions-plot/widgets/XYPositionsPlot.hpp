#pragma once

#include "xy-positions-plot/listeners/XYPositionsPlotListener.hpp"
#include "rfcommon/Vector.hpp"
#include "rfplot/RealtimePlot.hpp"

class QActionGroup;
class QwtPlotCurve;
class XYPositionsPlotModel;

class XYPositionsPlot 
    : public rfplot::RealtimePlot
    , public XYPositionsPlotListener
{
    Q_OBJECT

public:
    explicit XYPositionsPlot(XYPositionsPlotModel* model, QWidget* parent=nullptr);
    ~XYPositionsPlot();

    void clearCurves();
    void buildCurves();

public slots:
    void onDottedAction(bool enable);
    void onLinesAction(bool enable);
    void setCurveVisible(int sessionIdx, int fighterIdx, bool visible);

protected:
    void prependContextMenuActions(QMenu* menu) override;

private:
    void onDataSetChanged() override;
    void onDataChanged() override;
    void onNamesChanged() override;

private:
    XYPositionsPlotModel* model_;
    QActionGroup* curveTypeActionGroup_;
    rfcommon::Vector<QwtPlotCurve*> curves_;
};
