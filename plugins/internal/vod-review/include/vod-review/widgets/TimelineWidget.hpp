#pragma once

#include <QFrame>
#include <QVector>
#include <QColor>

class TimelineWidget : public QFrame
{
public:
    explicit TimelineWidget(QWidget* parent=nullptr);
    ~TimelineWidget();

    void clear();
    void setExtents(int start, int end);
    void addInterval(int start, int end);
    void addPoint(int offset);

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    struct Interval
    {
        int start;
        int end;
    };

    QVector<Interval> intervals_;
    QColor color_ = QColor(Qt::blue);
    int start_ = 0, end_ = 100;
};
