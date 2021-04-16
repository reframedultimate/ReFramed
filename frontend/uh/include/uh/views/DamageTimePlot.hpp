#pragma once

#include "uh/views/RealtimePlot.hpp"

class QwtPlotDirectPainter;
class QwtPlotCurve;

namespace uh {

class DamageTimePlot : public RealtimePlot
{
    Q_OBJECT
public:
    explicit DamageTimePlot(QWidget* parent=nullptr);
    ~DamageTimePlot();

public slots:
    void resetPlot(int numPlayers);
    void addPlayerDamageValue(int idx, uint32_t frame, float damage);
    void setPlayerName(int idx, const QString& tag);
    void replotAndAutoScale();

private:
    QVector<QwtPlotCurve*> curves_;
    float largestTimeSeen_ = 0.0;
};

}
