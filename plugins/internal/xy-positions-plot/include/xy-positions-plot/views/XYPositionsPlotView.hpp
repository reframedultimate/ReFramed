#pragma once

#include "xy-positions-plot/listeners/XYPositionsPlotListener.hpp"
#include "rfcommon/Vector.hpp"
#include <QWidget>

class QListView;
class QSplitter;
class XYPositionsPlotModel;
class XYPositionsPlot;

class XYPositionsPlotView
    : public QWidget
    , public XYPositionsPlotListener
{
public:
    explicit XYPositionsPlotView(XYPositionsPlotModel* model, QWidget* parent=nullptr);
    ~XYPositionsPlotView();

private:
    void onDataSetChanged() override;
    void onDataChanged() override;
    void onNamesChanged() override;

private:
    XYPositionsPlotModel* model_;
    QSplitter* splitter_;
    XYPositionsPlot* plot_;
    QListView* sessionsList_;
    int lastSessionCount_;
};
