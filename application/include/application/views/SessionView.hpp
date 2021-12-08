#pragma once

#include <QWidget>

namespace rfcommon {
    class Session;
}

namespace Ui {
    class SessionView;
}

namespace rfapp {

class DamageTimePlot;
class SessionDataView;
class XYPositionPlot;

class SessionView : public QWidget
{
    Q_OBJECT
public:
    explicit SessionView(QWidget* parent=nullptr);
    ~SessionView();

    void showDamagePlot();

public slots:
    void setSession(rfcommon::Session* session);
    void clear();

private slots:
    // This is still broken, see
    // https://www.qtcentre.org/threads/66591-QwtPlot-is-broken-(size-constraints-disregarded)
    void addPlotsToUI();

private:
    Ui::SessionView* ui_;
    DamageTimePlot* damageTimePlot_;
    XYPositionPlot* xyPositionPlot_;
    SessionDataView* sessionDataView_;
};

}
