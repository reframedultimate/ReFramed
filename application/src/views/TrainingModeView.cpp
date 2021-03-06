#include "application/ui_TrainingModeView.h"
#include "application/views/TrainingModeView.hpp"
#include "application/models/TrainingModeModel.hpp"
#include "application/models/CategoryModel.hpp"
#include "rfcommon/PluginInterface.hpp"
#include "rfcommon/RealtimePlugin.hpp"

namespace rfapp {

// ----------------------------------------------------------------------------
TrainingModeView::TrainingModeView(TrainingModeModel* trainingModel, CategoryModel* categoryModel, QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::TrainingModeView)
    , trainingModel_(trainingModel)
    , categoryModel_(categoryModel)
{
    ui_->setupUi(this);

    ui_->pushButton_launch->setEnabled(false);

    for (const auto& name : trainingModel_->availablePluginNames())
        ui_->listWidget_plugins->addItem(name);
    ui_->listWidget_plugins->sortItems();

    trainingModel_->dispatcher.addListener(this);
    categoryModel_->dispatcher.addListener(this);

    connect(ui_->listWidget_plugins, &QListWidget::currentTextChanged, this, &TrainingModeView::currentTextChanged);
    connect(ui_->pushButton_launch, &QPushButton::released, this, &TrainingModeView::launchPressed);
}

// ----------------------------------------------------------------------------
TrainingModeView::~TrainingModeView()
{
    // If there are still views created by plugins, have to destroy those now
    for (const auto& pluginName : trainingModel_->runningPluginNames())
    {
        trainingModel_->stopPlugin(pluginName);
    }

    categoryModel_->dispatcher.removeListener(this);
    trainingModel_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void TrainingModeView::currentTextChanged(const QString& text)
{
    const RFPluginFactoryInfo* info = trainingModel_->getPluginInfo(text);
    if (info == nullptr)
    {
        ui_->pushButton_launch->setEnabled(false);
        return;
    }

    ui_->lineEdit_name->setText(info->name);
    ui_->lineEdit_author->setText(info->author);
    ui_->lineEdit_contact->setText(info->contact);
    ui_->textBrowser_description->setText(info->description);

    ui_->pushButton_launch->setEnabled(true);
}

// ----------------------------------------------------------------------------
void TrainingModeView::launchPressed()
{
    QListWidgetItem* item = ui_->listWidget_plugins->currentItem();
    if (item == nullptr)
        goto fail;
    if (trainingModel_->launchPlugin(item->text()) == false)
        goto fail;

    return;

    fail : ui_->pushButton_launch->setEnabled(false);
}

// ----------------------------------------------------------------------------
void TrainingModeView::onTrainingModePluginLaunched(const QString& name, rfcommon::Plugin* model)
{
    QWidget* view = model->createView();
    if (view == nullptr)
        return;

    ui_->stackedWidget_runningPlugins->addWidget(view);
    views_.insert(model, view);

    categoryModel_->selectTrainingModePlugin(name);
    ui_->stackedWidget_top->setCurrentWidget(ui_->page_runningPlugins);
    ui_->stackedWidget_runningPlugins->setCurrentWidget(view);
}

// ----------------------------------------------------------------------------
void TrainingModeView::onTrainingModePluginStopped(const QString& name, rfcommon::Plugin* model)
{
    auto it = views_.find(model);
    assert(it != views_.end());
    QWidget* view = it.value();

    ui_->stackedWidget_runningPlugins->removeWidget(view);
    view->setParent(nullptr);
    model->destroyView(view);
    views_.erase(it);
}

// ----------------------------------------------------------------------------
void TrainingModeView::onCategorySelected(CategoryType category)
{
    if (category == CategoryType::TOP_LEVEL_TRAINING_MODE)
    {
        ui_->stackedWidget_top->setCurrentWidget(ui_->page_pluginLauncher);
    }
}

// ----------------------------------------------------------------------------
void TrainingModeView::onCategoryItemSelected(CategoryType category, const QString& name)
{
    if (category != CategoryType::TOP_LEVEL_TRAINING_MODE)
        return;

    rfcommon::Plugin* model = trainingModel_->runningPlugin(name);
    auto it = views_.find(model);
    assert(it != views_.end());
    QWidget* view = it.value();

    ui_->stackedWidget_top->setCurrentWidget(ui_->page_runningPlugins);
    ui_->stackedWidget_runningPlugins->setCurrentWidget(view);
}

}
