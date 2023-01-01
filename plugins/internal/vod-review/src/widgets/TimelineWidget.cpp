#include "vod-review/widgets/TimelineWidget.hpp"

#include <QPainter>

// ----------------------------------------------------------------------------
TimelineWidget::TimelineWidget(QWidget* parent)
    : QFrame(parent)
{
    setMinimumHeight(4);
}

// ----------------------------------------------------------------------------
TimelineWidget::~TimelineWidget()
{}

// ----------------------------------------------------------------------------
void TimelineWidget::clear()
{ 
    intervals_.clear();
    repaint();
}

// ----------------------------------------------------------------------------
void TimelineWidget::setExtents(int start, int end)
{ 
    start_ = start; 
    end_ = end;
    repaint();
}

// ----------------------------------------------------------------------------
void TimelineWidget::addInterval(int start, int end)
{ 
    intervals_.push_back({ start, end });
    repaint();
}

// ----------------------------------------------------------------------------
void TimelineWidget::addPoint(int offset)
{ 
    intervals_.push_back({ offset, offset });
    repaint();
}

// ----------------------------------------------------------------------------
void TimelineWidget::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    painter.setPen(QColor(40, 60, 60));
    painter.drawRect(0, 0, width() - 1, height() - 1);

    painter.setBrush(color_);
    for (const auto& interval : intervals_)
    {
        int start = end_ > start_ ? (interval.start - start_) * (width() - 1) / (end_ - start_) : 0;
        int end = end_ > start_ ? (interval.end - start_) * (width() - 1) / (end_ - start_) : 0;

        painter.drawRect(start, 2, end - start, height() - 5);
    }

    QFrame::paintEvent(e);
}
