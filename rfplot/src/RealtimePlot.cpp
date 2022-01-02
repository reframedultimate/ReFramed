#include "rfplot/RealtimePlot.hpp"
#include "rfplot/Panner.hpp"
#include "rfplot/Zoomer.hpp"
#include "rfplot/RectangleZoomer.hpp"
#include "rfplot/AutoScaler.hpp"

#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_legenditem.h>
#include <qwt_plot_textlabel.h>
#include <qwt_point_data.h>
#include <qwt_event_pattern.h>
#include <qwt_scale_engine.h>

#include <QKeyEvent>
#include <QAction>
#include <QMenu>
#include <QClipboard>
#include <QApplication>


namespace rfplot {

class RealtimePlotContextMenuStore
{
public:
    RealtimePlotContextMenuStore() :
        resetZoom       (NULL),
        copyImage       (NULL),
        copyCSV         (NULL),
        copyMatlabArray (NULL),
        saveAs          (NULL)
    {
        retranslate();
    }

    void retranslate()
    {
        resetZoom.setText(QWidget::tr("Reset Zoom"));
        copyImage.setText(QWidget::tr("Copy as image to clipboard"));
        copyCSV.setText(QWidget::tr("Copy as CSV to clipboard"));
        copyMatlabArray.setText(QWidget::tr("Copy as MATLAB array"));
        saveAs.setText(QWidget::tr("Save as..."));
    }

    QAction resetZoom;
    QAction copyImage;
    QAction copyCSV;
    QAction copyMatlabArray;
    QAction saveAs;
};

// ----------------------------------------------------------------------------
RealtimePlot::RealtimePlot(QWidget* parent) :
    QwtPlot(parent),
    contextMenuStore_(new RealtimePlotContextMenuStore),
    autoScaler_(NULL),
    lastScaleWasAutomatic_(true)
{
    // set up background colours
    setCanvasBackground(QColor(Qt::white));

    // Add a dotted grid
    QwtPlotGrid* plotGrid = new QwtPlotGrid;
    plotGrid->enableX(true);
    plotGrid->enableY(true);
    plotGrid->setMajorPen(QPen(Qt::black, 0, Qt::DotLine));
    plotGrid->setMinorPen(QPen(Qt::gray,  0, Qt::DotLine));
    plotGrid->attach(this);

    // Display current x/y values using a tracker
    //unitTracker_ = new UnitTracker(canvas());

    // Pan the plot by dragging MMB
    Panner* panner = new Panner(canvas());
    panner->setMousePattern(QwtEventPattern::MouseSelect1, Qt::MiddleButton);

    // Zoom the plot by dragging RMB
    Zoomer* zoomer = new Zoomer(canvas());
    zoomer->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton);

    // Auto-fit data by double-clicking
    autoScaler_ = new AutoScaler(canvas());
    autoScaler_->setMouseButton(Qt::RightButton | Qt::LeftButton);

    // Click and drag a box to zoom in on a specific area
    RectangleZoomer* rectangleZoomer = new RectangleZoomer(canvas());
    rectangleZoomer->setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton);

    // Track left clicks and drags in plot
    DeltaPlotPicker* picker = new DeltaPlotPicker(canvas());
    picker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton);
    connect(picker, &DeltaPlotPicker::activated, this, &RealtimePlot::onPickerActivated);
    connect(picker, &DeltaPlotPicker::moved, this, &RealtimePlot::onPickerMoved);

    // Don't show context menu if right click was dragged
    DeltaPlotPicker* trackRightMouseDrag = new DeltaPlotPicker(canvas());
    trackRightMouseDrag->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton);
    connect(trackRightMouseDrag, SIGNAL(moved(QPointF,QPointF)), this, SLOT(cancelContextMenu()));

    // We want to auto-fit data without leaving any gaps
    axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating, true);
    axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Floating, true);

    // By default, auto-scaling should be turned off.
    setAxisAutoScale(QwtPlot::xBottom, false);
    setAxisAutoScale(QwtPlot::yLeft, false);

    // Set up plot context menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
    connect(&contextMenuStore_->resetZoom, SIGNAL(triggered()), this, SLOT(resetZoom()));
    connect(&contextMenuStore_->copyImage, SIGNAL(triggered()), this, SLOT(copyImageToClipboard()));
    connect(&contextMenuStore_->copyCSV, SIGNAL(triggered()), this, SLOT(copyCSVToClipboard()));
    connect(&contextMenuStore_->copyMatlabArray, SIGNAL(triggered()), this, SLOT(copyMatlabToClipboard()));
    connect(&contextMenuStore_->saveAs, SIGNAL(triggered()), this, SLOT(saveAs()));
}

// ----------------------------------------------------------------------------
RealtimePlot::~RealtimePlot()
{
    delete contextMenuStore_;
}

// ----------------------------------------------------------------------------
void RealtimePlot::forceAutoScale()
{
    autoScaler_->autoScale();
    lastScaleWasAutomatic_ = true;
}

// ----------------------------------------------------------------------------
void RealtimePlot::conditionalAutoScale()
{
    if (lastScaleWasAutomatic_)
        autoScaler_->autoScale();
    else
        replot();
}

// ----------------------------------------------------------------------------
/*
void RealtimePlot::setAxisUnits(QString unitX, QString unitY)
{
    setXAxisUnit(unitX);
    setYAxisUnit(unitY);
}

// ----------------------------------------------------------------------------
void RealtimePlot::setXAxisUnit(QString unitX)
{
    unitTracker_->setXUnit(unitX);
}

// ----------------------------------------------------------------------------
void RealtimePlot::setYAxisUnit(QString unitY)
{
    unitTracker_->setYUnit(unitY);
}*/

// ----------------------------------------------------------------------------
void RealtimePlot::changeEvent(QEvent* e)
{
    if(e->type() == QEvent::LanguageChange)
    {
        contextMenuStore_->retranslate();
    }

    QWidget::changeEvent(e);
}

// ----------------------------------------------------------------------------
bool RealtimePlot::event(QEvent* event)
{
    /*
     * This function auto-scales the plot when the user presses the space bar.
     * I may make this configurable in the future, not sure yet.
     */
    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
        if(ke->key() == Qt::Key_Space)
        {
            forceAutoScale();
            return true;
        }
    }

    /*
     * Unfortunately, Qt context menus are activated when the mouse *presses*.
     * With the plot, we really want to activate it when the mouse is
     * *released*.
     *
     * The following code delays the right mouse click until it is released.
     * We then emit customContextMenuRequested() manually. Note that if the
     * user right-clicks and drags in the plot, the context menu is never
     * shown.
     */
    if(event->type() == QEvent::ContextMenu)
    {
        QContextMenuEvent* e = static_cast<QContextMenuEvent*>(event);
        contextMenuRequestedAt_ = e->pos();
        return true;
    }
    if(event->type() == QEvent::MouseButtonRelease && !contextMenuRequestedAt_.isNull())
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if(me->button() == Qt::RightButton)
        {
            QPoint pos = contextMenuRequestedAt_;
            contextMenuRequestedAt_ = QPoint();
            if(contextMenuPolicy() == Qt::CustomContextMenu)
                emit customContextMenuRequested(pos);
            return true;
        }
    }

    // Any click in the plot is considered to "dirty" the results of an auto-scale.
    if(event->type() == QEvent::MouseButtonPress)
    {
        lastScaleWasAutomatic_ = false;
    }

    if(event->type() == QEvent::MouseButtonDblClick)
    {
        lastScaleWasAutomatic_ = true;
    }

    return QwtPlot::event(event);
}

// ----------------------------------------------------------------------------
void RealtimePlot::onPickerActivated(bool activated, const QPointF& point)
{
    if(activated)
    {
        onPickerMoved(point, point);
    }
}

// ----------------------------------------------------------------------------
void RealtimePlot::onPickerMoved(const QPointF& origin, const QPointF& current)
{
    (void)origin;
    // Whenever left mouse button is pressed or dragged
    emit pointSelected(current);
}

// ----------------------------------------------------------------------------
void RealtimePlot::showContextMenu(const QPoint& pos)
{
    (void)pos;
    QMenu contextMenu(tr("Context Menu"));

    prependContextMenuActions(&contextMenu);

    contextMenu.addAction(&contextMenuStore_->resetZoom);
    contextMenu.addAction(&contextMenuStore_->copyImage);
    contextMenu.addAction(&contextMenuStore_->copyCSV);
    contextMenu.addAction(&contextMenuStore_->copyMatlabArray);
    // TODO contextMenu.addAction(&contextMenuStore_->saveAs);

    contextMenu.exec(mapToGlobal(pos));
}

// ----------------------------------------------------------------------------
void RealtimePlot::cancelContextMenu()
{
    contextMenuRequestedAt_ = QPoint();
}

// ----------------------------------------------------------------------------
void RealtimePlot::resetZoom()
{
    forceAutoScale();
}

// ----------------------------------------------------------------------------
void RealtimePlot::copyImageToClipboard()
{
    // It's nicer to have a white background for word documents and such
    QPalette restorePal = palette();
    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::white);
    setPalette(pal);

    QImage img(this->size(), QImage::Format_ARGB32);
    QPainter painter(&img);
    QwtPlotRenderer renderer;
    renderer.render(this, &painter, this->rect());
    QApplication::clipboard()->setImage(img);

    setPalette(restorePal);
}

// ----------------------------------------------------------------------------
void RealtimePlot::extractCurveData(QVector<QVector<QString>>* container, bool writeTitles)
{
    const QwtPlotItemList& items = itemList(QwtPlotItem::Rtti_PlotCurve);
    for(QwtPlotItemList::const_iterator it = items.begin(); it != items.end(); ++it)
    {
        // Skip if the item doesn't contain QwtPointArrayData
        QwtPlotCurve* curve = static_cast<QwtPlotCurve*>(*it);
        QwtArraySeriesData<QPointF>* series = dynamic_cast<QwtArraySeriesData<QPointF>*>(curve->data());
            if(!series)
                continue;

        // Write X axis data
        QVector<QString> samplesStrXAxis;
        if(writeTitles)
            samplesStrXAxis.push_back(curve->plot()->title().text() + ": " + curve->title().text() + " (X axis)");
        for(const auto& p : series->samples())
            samplesStrXAxis.push_back(QString::number(p.x(), 'g', 17));

        // Write Y axis data
        QVector<QString> samplesStrYAxis;
        if(writeTitles)
            samplesStrYAxis.push_back(curve->plot()->title().text() + ": " + curve->title().text() + " (Y axis)");
        for(const auto& p : series->samples())
            samplesStrYAxis.push_back(QString::number(p.y(), 'g', 17));

        container->push_back(samplesStrXAxis);
        container->push_back(samplesStrYAxis);
    }
}

// ----------------------------------------------------------------------------
void RealtimePlot::copyCSVToClipboard()
{
    QVector<QVector<QString> > itemSamples;
    extractCurveData(&itemSamples, true);
    if(itemSamples.size() == 0)
        return;

    // Re-arrange data so the samples are saved as columns.
    QString text;
    int samplesCount = itemSamples[0].size();
    for(int i = 0; i != samplesCount; ++i)
    {
        for(QVector<QVector<QString> >::const_iterator it = itemSamples.begin(); it != itemSamples.end(); ++it)
        {
            if(samplesCount < it->size())
                samplesCount = it->size();

            if(i >= it->size())
                text += "\"\",";
            else
                text += "\"" + (*it)[i] + "\",";
        }
        text[text.length() - 1] = QChar('\n');
    }

    QApplication::clipboard()->setText(text);
}

// ----------------------------------------------------------------------------
void RealtimePlot::copyMatlabToClipboard()
{
    QVector<QVector<QString>> itemSamples;
    extractCurveData(&itemSamples);
    if(itemSamples.size() == 0)
        return;

    QString text = "{";
    for(QVector<QVector<QString> >::const_iterator it = itemSamples.begin(); it != itemSamples.end(); ++it)
    {
        text += "[";
        for(QVector<QString>::const_iterator it2 = it->begin(); it2 != it->end(); ++it2)
            text += *it2 + ",";
        text[text.length() - 1] = QChar(']');
        text += ",";
    }
    text[text.length() - 1] = QChar('}');
    text += ";";

    QApplication::clipboard()->setText(text);
}

// ----------------------------------------------------------------------------
void RealtimePlot::saveAs()
{
}

} // namespace uh
