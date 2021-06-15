#include "uhplot/ColorPalette.hpp"
#include "application/views/XYPositionPlot.hpp"
#include "uh/Recording.hpp"
#include "uh/PlayerState.hpp"
#include "qwt_plot_curve.h"
#include "qwt_date_scale_draw.h"
#include <QMenu>
#include <QAction>
#include <QSignalMapper>

namespace uhapp {

namespace {

class CurveData : public QwtArraySeriesData<QPointF>
{
public:
    QRectF boundingRect() const override
    {
        if (d_boundingRect.width() < 0.0)
            d_boundingRect = qwtBoundingRect(*this);

        return d_boundingRect;
    }

    inline void append(const QPointF& point)
    {
        d_samples += point;
        d_boundingRect = QRectF(0.0, 0.0, -1.0, -1.0);
    }

    inline void setSample(int idx, const QPointF& point)
    {
        d_samples[idx] = point;
        d_boundingRect = QRectF(0.0, 0.0, -1.0, -1.0);
    }

    void clear()
    {
        d_samples.clear();
        d_samples.squeeze();
        d_boundingRect = QRectF(0.0, 0.0, -1.0, -1.0);
    }
};

}

// ----------------------------------------------------------------------------
XYPositionPlot::XYPositionPlot(QWidget* parent)
    : RealtimePlot(parent)
    , curveTypeActionGroup_(new QActionGroup(this))
{
    setTitle("XY Positions");

    QAction* dotted = curveTypeActionGroup_->addAction("Dotted");
    dotted->setCheckable(true);
    dotted->setChecked(true);
    QAction* lines = curveTypeActionGroup_->addAction("Lines");
    lines->setCheckable(true);

    connect(dotted, &QAction::triggered, this, &XYPositionPlot::onDottedAction);
    connect(lines, &QAction::triggered, this, &XYPositionPlot::onLinesAction);
}

// ----------------------------------------------------------------------------
XYPositionPlot::~XYPositionPlot()
{
    clear();
}

// ----------------------------------------------------------------------------
void XYPositionPlot::clear()
{
    if (recording_)
        recording_->dispatcher.removeListener(this);
    recording_ = nullptr;

    for (auto& curve : curves_)
        delete curve;
    curves_.clear();
    replot();
}

// ----------------------------------------------------------------------------
void XYPositionPlot::setRecording(uh::Recording* recording)
{
    clear();
    recording_ = recording;
    recording_->dispatcher.addListener(this);

    for (int player = 0; player != recording_->playerCount(); ++player)
    {
        CurveData* data = new CurveData;
        QwtPlotCurve* curve = new QwtPlotCurve;
        curve->setPen(QPen(uhplot::ColorPalette::getColor(player), 2.0));
        curve->setData(data);
        curve->setTitle(recording_->playerName(player).cStr());
        curve->setStyle(curveTypeActionGroup_->actions()[0]->isChecked() ? QwtPlotCurve::Dots : QwtPlotCurve::Lines);
        curve->attach(this);
        curves_.push_back(curve);

        for (int i = 0; i < recording_->playerStateCount(player); ++i)
        {
            const auto& state = recording_->playerStateAt(player, i);
            data->append(QPointF(state.posx(), state.posy()));
        }
    }

    forceAutoScale();
}

// ----------------------------------------------------------------------------
void XYPositionPlot::prependContextMenuActions(QMenu* menu)
{
    for (const auto& action : curveTypeActionGroup_->actions())
        menu->addAction(action);
    menu->addSeparator();

    if (recording_ == nullptr)
        return;
    for (int i = 0; i != recording_->playerCount(); ++i)
    {
        QAction* a = menu->addAction(recording_->playerName(i).cStr());
        a->setCheckable(true);
        a->setChecked(curves_[i]->isVisible());
        connect(a, &QAction::triggered, [=](bool checked) {
            setCurveVisible(i, checked);
        });
    }
    menu->addSeparator();
}

// ----------------------------------------------------------------------------
void XYPositionPlot::onActiveRecordingPlayerNameChanged(int player, const uh::SmallString<15>& name)
{
    curves_[player]->setTitle(name.cStr());
}

// ----------------------------------------------------------------------------
void XYPositionPlot::onActiveRecordingNewUniquePlayerState(int player, const uh::PlayerState& state)
{
    CurveData* data = static_cast<CurveData*>(curves_[player]->data());
    data->append(QPointF(state.posx(), state.posy()));
    conditionalAutoScale();
}

// ----------------------------------------------------------------------------
void XYPositionPlot::onDottedAction(bool enable)
{
    if (!enable)
        return;

    for (auto& curve : curves_)
        curve->setStyle(QwtPlotCurve::Dots);

    replot();
}

// ----------------------------------------------------------------------------
void XYPositionPlot::onLinesAction(bool enable)
{
    if (!enable)
        return;

    for (auto& curve : curves_)
        curve->setStyle(QwtPlotCurve::Lines);

    replot();
}

// ----------------------------------------------------------------------------
void XYPositionPlot::setCurveVisible(int player, bool visible)
{
    curves_[player]->setVisible(visible);
    replot();
}

}
