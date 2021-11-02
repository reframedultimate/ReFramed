#pragma once

#include <QWidget>

namespace Ui {
    class TrainingModeView;
}

namespace uhapp {

class PluginManager;
class TrainingMode;

class TrainingModeView : public QWidget
{
    Q_OBJECT
public:
    TrainingModeView(TrainingMode* training, PluginManager* pluginManager, QWidget* parent=nullptr);
    ~TrainingModeView();

private slots:
    void currentTextChanged(const QString& text);
    void launchPressed();

private:
    Ui::TrainingModeView* ui_;
    TrainingMode* training_;
    PluginManager* pluginManager_;
};

}
