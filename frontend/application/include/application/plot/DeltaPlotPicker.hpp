#pragma once

#include "qwt_plot_picker.h"
#include "qwt_scale_map.h"

namespace uh {

/*!
 * Qwt provides QwtPlotPicker, which gives us absolute positions in the plot
 * in pixel space.
 *
 * Our controls require relative mouse movement in the axis transformed space.
 * This class is tailored towards these needs. It provides a way to track delta
 * movement of the mouse, and it also provides the absolute positions of where
 * the mouse initially clicked and where it currently is (in transformed space)
 *
 * The PlotPanner, PlotZoomer and PlotAutoScaler controls all rely on this.
 */
class DeltaPlotPicker : public QwtPlotPicker
{
    Q_OBJECT

public:
    explicit DeltaPlotPicker(QWidget* canvas);
    virtual ~DeltaPlotPicker();

    QPointF originalInvTransform(const QPoint& point) const;

signals:
    void activated(bool activated, const QPointF& point);

    //! When the mouse moves, its delta movement can be obtained using this.
    void delta(const QPointF& delta);

    /*!
     * When the mouse moves, the original position where the mouse initially
     * clicked as well as its current position can be obtained.
     */
    void moved(const QPointF& origin, const QPointF& current);

    /*!
     * When the mouse is released, the original position where the mouse
     * initially clicked as well as the position where it was released can be
     * obtained.
     */
    void released(const QPointF& origin, const QPointF& current);

private slots:
    void onActivated(bool activated);
    void onMoved(const QPoint& point);

private:
    // We want to modify the behaviour of QwtPlot's move()
    virtual void move(const QPoint& point);

    QPoint lastPoint_;
    QPointF originalClickPosition_;
    QwtScaleMap originalCanvasMapX_;
    QwtScaleMap originalCanvasMapY_;
    bool userJustClicked_;
};

} // namespace uh
