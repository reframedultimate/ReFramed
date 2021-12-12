#pragma once

#include "xy-positions-plot/listeners/XYPositionsPlotListener.hpp"
#include "rfplot/RealtimePlot.hpp"

class QActionGroup;
class QwtPlotDirectPainter;
class QwtPlotCurve;

class XYPositionPlotContextMenuActions;
class XYPositionsPlotModel;

class XYPositionsPlotView : public rfplot::RealtimePlot
                          , public XYPositionsPlotListener
{
    Q_OBJECT

public:
    explicit XYPositionsPlotView(XYPositionsPlotModel* model, QWidget* parent=nullptr);
    ~XYPositionsPlotView();

private:
    void clearUI();

private:
    void onXYPositionsPlotSessionSet(rfcommon::Session* session) override;
    void onXYPositionsPlotSessionCleared(rfcommon::Session* session) override;
    void onXYPositionsPlotNameChanged(int playerIdx, const rfcommon::SmallString<15>& name) override;
    void onXYPositionsPlotNewValue(int playerIdx, float posx, float posy) override;

protected:
    void prependContextMenuActions(QMenu* menu) override;

private slots:
    void onDottedAction(bool enable);
    void onLinesAction(bool enable);
    void setCurveVisible(int player, bool visible);

private:
    XYPositionsPlotModel* model_;
    QVector<QwtPlotCurve*> curves_;
    QActionGroup* curveTypeActionGroup_;
};
