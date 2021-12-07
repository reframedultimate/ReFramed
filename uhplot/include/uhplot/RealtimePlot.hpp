#pragma once

#include "uhplot/config.hpp"
#include <qwt_plot.h>

class QMenu;

namespace uhplot {

class AutoScaler;
//class UnitTracker;
class RealtimePlotContextMenuStore;
class RealtimePlotListener;

/*!
 *
 */
class UHPLOT_PUBLIC_API RealtimePlot : public QwtPlot
{
    Q_OBJECT

public:
    explicit RealtimePlot(QWidget* parent = 0);
    virtual ~RealtimePlot();

    void forceAutoScale();
    void conditionalAutoScale();

    /*void setAxisUnits(QString unitX, QString unitY);
    void setXAxisUnit(QString unitX);
    void setYAxisUnit(QString unitY);*/

signals:
    /*!
     * The mouse position in plot coordinate space is emitted when the user
     * clicks and drags the mouse.
     */
    void pointSelected(QPointF point);

protected:
    virtual void changeEvent(QEvent* e) override;
    virtual bool event(QEvent* event) override;
    virtual void prependContextMenuActions(QMenu* menu) { (void)menu; }

private slots:
    void onPickerActivated(bool activated, const QPointF& point);
    // Whenever left mouse button is pressed or dragged
    void onPickerMoved(const QPointF& origin, const QPointF& current);
    // Plot context menu handlers
    void showContextMenu(const QPoint& pos);
    void cancelContextMenu();
    void resetZoom();
    void copyImageToClipboard();
    void extractCurveData(QVector<QVector<QString> >* container, bool writeTitles=false);
    void copyCSVToClipboard();
    void copyMatlabToClipboard();
    void saveAs();

private:
    RealtimePlotContextMenuStore* contextMenuStore_;
    AutoScaler* autoScaler_;
    //UnitTracker* unitTracker_;
    QPointF activatedPoint_;
    QPoint contextMenuRequestedAt_;
    bool lastScaleWasAutomatic_;
};

} // namespace uh
