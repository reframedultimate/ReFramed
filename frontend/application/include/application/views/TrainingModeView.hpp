#pragma once

#include "application/listeners/TrainingModeListener.hpp"
#include "application/listeners/CategoryListener.hpp"
#include <QWidget>
#include <QHash>

namespace Ui {
    class TrainingModeView;
}

namespace uhapp {

class CategoryModel;
class TrainingModeModel;

class TrainingModeView : public QWidget
                       , public TrainingModeListener
                       , public CategoryListener
{
    Q_OBJECT

public:
    TrainingModeView(TrainingModeModel* trainingModel, CategoryModel* categoryModel, QWidget* parent=nullptr);
    ~TrainingModeView();

private slots:
    void currentTextChanged(const QString& text);
    void launchPressed();

private:
    void onTrainingModePluginLaunched(const QString& name, uh::Plugin* plugin) override;
    void onTrainingModePluginStopped(const QString& name, uh::Plugin* plugin) override;

private:
    void onCategorySelected(CategoryType category) override;
    void onCategoryItemSelected(CategoryType category, const QString& name) override;

private:
    struct PluginView
    {
        uh::Plugin* plugin;
        QWidget* view;
    };

    Ui::TrainingModeView* ui_;
    TrainingModeModel* trainingModel_;
    CategoryModel* categoryModel_;
    QHash<QString, PluginView> pluginViews_;
};

}
