#pragma once

#include <QWidget>

namespace rfcommon {
    class Session;
    class RealtimePlugin;
}

namespace Ui {
    class SessionView;
}

namespace rfapp {

class PluginManager;
class SessionDataView;

class SessionView : public QWidget
{
    Q_OBJECT
public:
    explicit SessionView(PluginManager* pluginManager, QWidget* parent=nullptr);
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
    PluginManager* pluginManager_;
    Ui::SessionView* ui_;
    rfcommon::RealtimePlugin* damageTimePlugin_ = nullptr;
    QWidget* damageTimePlot_ = nullptr;
    rfcommon::RealtimePlugin* xyPositionsPlugin_ = nullptr;
    QWidget* xyPositionPlot_ = nullptr;
    SessionDataView* sessionDataView_;
};

}
