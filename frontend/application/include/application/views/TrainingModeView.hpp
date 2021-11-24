#pragma once

#include "application/listeners/TrainingModeListener.hpp"
#include <QWidget>
#include <QHash>

namespace Ui {
    class TrainingModeView;
}

namespace uhapp {

class TrainingModeModel;

class TrainingModeView : public QWidget,
                         public TrainingModeListener
{
    Q_OBJECT

public:
    TrainingModeView(TrainingModeModel* model, QWidget* parent=nullptr);
    ~TrainingModeView();

private slots:
    void currentTextChanged(const QString& text);
    void launchPressed();

private:
    void onTrainingModePluginLaunched(const QString& name, uh::RealtimePlugin* plugin) override;
    void onTrainingModePluginStopped(const QString& name, uh::RealtimePlugin* plugin) override;

private:
    Ui::TrainingModeView* ui_;
    TrainingModeModel* model_;
    QHash<uh::RealtimePlugin*, QWidget*> pluginViews_;
};

}
