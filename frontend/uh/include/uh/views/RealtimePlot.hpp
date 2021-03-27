#pragma once

#include <qwt_plot.h>

namespace uh {

class AutoScaler;
//class UnitTracker;
class RealtimePlotContextMenuStore;
class RealtimePlotListener;

/*!
 *
 */
class RealtimePlot : public QwtPlot
{
    Q_OBJECT

public:
    explicit RealtimePlot(QWidget* parent = 0);
    virtual ~RealtimePlot();

    void autoScale();

    /*void setAxisUnits(QString unitX, QString unitY);
    void setXAxisUnit(QString unitX);
    void setYAxisUnit(QString unitY);*/

    bool lastScaleWasAutomatic() const;

signals:
    /*!
     * The mouse position in plot coordinate space is emitted when the user
     * clicks and drags the mouse.
     */
    void pointSelected(QPointF point);

protected:
    virtual void changeEvent(QEvent* e) override;
    virtual bool event(QEvent* event) override;
    virtual QSize sizeHint() const override;

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
    bool contextMenuWasRequested_;
    bool lastScaleWasAutomatic_;
};

} // namespace uh
