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
    void setSavedGameSession(rfcommon::Session* session);
    void clearSavedGameSession(rfcommon::Session* session);

    void setSavedGameSessionSet(rfcommon::Session** sessions, int count);
    void clearSavedGameSessionSet(rfcommon::Session** sessions, int count);

private slots:
    // This is still broken, see
    // https://www.qtcentre.org/threads/66591-QwtPlot-is-broken-(size-constraints-disregarded)
    void addPlotsToUI();

private:
    PluginManager* pluginManager_;
    Ui::SessionView* ui_;
    
    struct PluginData
    {
        rfcommon::RealtimePlugin* plugin;
        QWidget* view;
        QString name;
    };

    QVector<PluginData> plugins_;
};

}
