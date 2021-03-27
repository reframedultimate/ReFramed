#pragma once

#include "uh/views/RealtimePlot.hpp"

class QwtPlotDirectPainter;
class QwtPlotCurve;

namespace uh {

class DamagePlot : public RealtimePlot
{
    Q_OBJECT
public:
    explicit DamagePlot(QWidget* parent=nullptr);
    ~DamagePlot();

public slots:
    void resetPlot(int numPlayers);
    void addPlayerDamageValue(int idx, uint32_t frame, float damage);
    void setPlayerTag(int idx, const QString& tag);
    void replotAndAutoScale();

private:
    QVector<QwtPlotCurve*> curves_;
    float largestTimeSeen_ = 0.0;
};

}
