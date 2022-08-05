#pragma once

#include <QTabWidget>

namespace rfcommon {
    class Session;
    class RealtimePlugin;
}

namespace rfapp {

class PluginManager;
class SessionDataView;

class SessionView : public QTabWidget
{
    Q_OBJECT
public:
    explicit SessionView(PluginManager* pluginManager, QWidget* parent=nullptr);
    ~SessionView();

public slots:
    void setSavedGameSession(rfcommon::Session* session);
    void clearSavedGameSession(rfcommon::Session* session);

    void setSavedGameSessionSet(rfcommon::Session** sessions, int count);
    void clearSavedGameSessionSet(rfcommon::Session** sessions, int count);

private slots:
    void onTabBarClicked(int index);
    void onCurrentTabChanged(int index);

private:
    PluginManager* pluginManager_;
    
    struct PluginData
    {
        rfcommon::RealtimePlugin* plugin;
        QWidget* view;
        QString name;
    };

    QVector<PluginData> plugins_;
    int previousTab_;
};

}
