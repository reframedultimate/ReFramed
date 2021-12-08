#pragma once

#include "application/listeners/TrainingModeListener.hpp"
#include "application/listeners/CategoryListener.hpp"
#include <QWidget>
#include <QHash>

namespace Ui {
    class TrainingModeView;
}

namespace rfapp {

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
    void onTrainingModePluginLaunched(const QString& name, rfcommon::Plugin* plugin) override;
    void onTrainingModePluginStopped(const QString& name, rfcommon::Plugin* plugin) override;

private:
    void onCategorySelected(CategoryType category) override;
    void onCategoryItemSelected(CategoryType category, const QString& name) override;

private:
    Ui::TrainingModeView* ui_;
    TrainingModeModel* trainingModel_;
    CategoryModel* categoryModel_;
    QHash<rfcommon::Plugin*, QWidget*> views_;
};

}
