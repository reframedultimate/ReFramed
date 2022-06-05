#include "xy-positions-plot/views/XYPositionsPlotView.hpp"
#include "xy-positions-plot/models/XYPositionsPlotModel.hpp"
#include "rfplot/ColorPalette.hpp"
#include "rfcommon/SavedGameSession.hpp"
#include "rfcommon/Frame.hpp"
#include "qwt_plot_curve.h"
#include "qwt_date_scale_draw.h"
#include <QMenu>
#include <QAction>
#include <QSignalMapper>
#include <QPen>

namespace {

class CurveData : public QwtArraySeriesData<QPointF>
{
public:
    QRectF boundingRect() const override
    {
        if (cachedBoundingRect.width() < 0.0)
            cachedBoundingRect = qwtBoundingRect(*this);

        return cachedBoundingRect;
    }

    inline void append(const QPointF& point)
    {
        m_samples += point;
        cachedBoundingRect = QRectF(0.0, 0.0, -1.0, -1.0);
    }

    inline void setSample(int idx, const QPointF& point)
    {
        m_samples[idx] = point;
        cachedBoundingRect = QRectF(0.0, 0.0, -1.0, -1.0);
    }

    void clear()
    {
        m_samples.clear();
        m_samples.squeeze();
        cachedBoundingRect = QRectF(0.0, 0.0, -1.0, -1.0);
    }
};

}

// ----------------------------------------------------------------------------
XYPositionsPlotView::XYPositionsPlotView(XYPositionsPlotModel* model, QWidget* parent)
    : RealtimePlot(parent)
    , model_(model)
    , curveTypeActionGroup_(new QActionGroup(this))
{
    setTitle("XY Positions");

    QAction* dotted = curveTypeActionGroup_->addAction("Dotted");
    dotted->setCheckable(true);
    QAction* lines = curveTypeActionGroup_->addAction("Lines");
    lines->setCheckable(true);
    lines->setChecked(true);

    if (model_->session())
        XYPositionsPlotView::onXYPositionsPlotSessionSet(model_->session());

    model_->dispatcher.addListener(this);

    connect(dotted, &QAction::triggered, this, &XYPositionsPlotView::onDottedAction);
    connect(lines, &QAction::triggered, this, &XYPositionsPlotView::onLinesAction);
}

// ----------------------------------------------------------------------------
XYPositionsPlotView::~XYPositionsPlotView()
{
    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void XYPositionsPlotView::clearUI()
{
    for (auto& curve : curves_)
        delete curve;
    curves_.clear();
}

// ----------------------------------------------------------------------------
void XYPositionsPlotView::onXYPositionsPlotSessionSet(rfcommon::Session* session)
{
    clearUI();

    for (int player = 0; player != session->fighterCount(); ++player)
    {
        CurveData* data = new CurveData;
        QwtPlotCurve* curve = new QwtPlotCurve;
        const QList<QAction*>& actions = curveTypeActionGroup_->actions();
        curve->setPen(QPen(rfplot::ColorPalette::getColor(player), 2.0));
        curve->setData(data);
        curve->setTitle(session->name(player).cStr());
        curve->setStyle(actions[0]->isChecked() ? QwtPlotCurve::Dots : QwtPlotCurve::Lines);
        curve->attach(this);
        curves_.push_back(curve);

        for (int i = 0; i < session->frameCount(); ++i)
        {
            const auto& state = session->state(i, player);
            data->append(QPointF(state.posx(), state.posy()));
        }
    }

    forceAutoScale();
}

// ----------------------------------------------------------------------------
void XYPositionsPlotView::onXYPositionsPlotSessionCleared(rfcommon::Session* session)
{
    (void)session;
}

// ----------------------------------------------------------------------------
void XYPositionsPlotView::prependContextMenuActions(QMenu* menu)
{
    for (const auto& action : curveTypeActionGroup_->actions())
        menu->addAction(action);
    menu->addSeparator();

    rfcommon::Session* session = model_->session();
    if (session == nullptr)
        return;

    for (int i = 0; i != session->fighterCount(); ++i)
    {
        QAction* a = menu->addAction(session->name(i).cStr());
        a->setCheckable(true);
        a->setChecked(curves_[i]->isVisible());
        connect(a, &QAction::triggered, [=](bool checked) {
            setCurveVisible(i, checked);
        });
    }
    menu->addSeparator();
}

// ----------------------------------------------------------------------------
void XYPositionsPlotView::onXYPositionsPlotNameChanged(int playerIdx, const rfcommon::SmallString<15>& name)
{
    curves_[playerIdx]->setTitle(name.cStr());
}

// ----------------------------------------------------------------------------
void XYPositionsPlotView::onXYPositionsPlotNewValue(const rfcommon::SmallVector<float, 8>& posx, const rfcommon::SmallVector<float, 8>& posy)
{
    for (int playerIdx = 0; playerIdx != posx.count(); ++playerIdx)
    {
        CurveData* data = static_cast<CurveData*>(curves_[playerIdx]->data());
        data->append(QPointF(posx[playerIdx], posy[playerIdx]));
    }

    conditionalAutoScale();
}

// ----------------------------------------------------------------------------
void XYPositionsPlotView::onDottedAction(bool enable)
{
    if (!enable)
        return;

    for (auto& curve : curves_)
        curve->setStyle(QwtPlotCurve::Dots);

    replot();
}

// ----------------------------------------------------------------------------
void XYPositionsPlotView::onLinesAction(bool enable)
{
    if (!enable)
        return;

    for (auto& curve : curves_)
        curve->setStyle(QwtPlotCurve::Lines);

    replot();
}

// ----------------------------------------------------------------------------
void XYPositionsPlotView::setCurveVisible(int player, bool visible)
{
    curves_[player]->setVisible(visible);
    replot();
}
