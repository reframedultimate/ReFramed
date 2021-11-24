#include "application/ui_TrainingModeView.h"
#include "application/views/TrainingModeView.hpp"
#include "application/models/TrainingModeModel.hpp"
#include "uh/PluginInterface.hpp"
#include "uh/RealtimePlugin.hpp"

namespace uhapp {

// ----------------------------------------------------------------------------
TrainingModeView::TrainingModeView(TrainingModeModel* model, QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::TrainingModeView)
    , model_(model)
{
    ui_->setupUi(this);

    ui_->pushButton_launch->setEnabled(false);

    for (const auto& name : model_->availablePluginNames())
        ui_->listWidget_plugins->addItem(name);

    model_->dispatcher.addListener(this);

    connect(ui_->listWidget_plugins, &QListWidget::currentTextChanged, this, &TrainingModeView::currentTextChanged);
    connect(ui_->pushButton_launch, &QPushButton::released, this, &TrainingModeView::launchPressed);
}

// ----------------------------------------------------------------------------
TrainingModeView::~TrainingModeView()
{
    // If there are still views created by plugins, have to destroy those now
    for (auto it = pluginViews_.begin(); it != pluginViews_.end(); ++it)
    {
        uh::RealtimePlugin* plugin = it.key();
        QWidget* widget = it.value();

        ui_->stackedWidget_runningPlugins->removeWidget(widget);
        widget->setParent(nullptr);
        plugin->destroyView(widget);
    }

    model_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void TrainingModeView::currentTextChanged(const QString& text)
{
    const UHPluginInfo* info = model_->getPluginInfo(text);
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
    if (model_->launchPlugin(item->text()) == false)
        goto fail;

    return;

    fail : ui_->pushButton_launch->setEnabled(false);
}

// ----------------------------------------------------------------------------
void TrainingModeView::onTrainingModePluginLaunched(const QString& name, uh::RealtimePlugin* plugin)
{
    QWidget* widget = plugin->createView();
    if (widget == nullptr)
        return;

    pluginViews_.insert(plugin, widget);
    ui_->stackedWidget_runningPlugins->addWidget(widget);

    ui_->stackedWidget_top->setCurrentWidget(ui_->page_runningPlugins);
    ui_->stackedWidget_runningPlugins->setCurrentWidget(widget);
}

// ----------------------------------------------------------------------------
void TrainingModeView::onTrainingModePluginStopped(const QString& name, uh::RealtimePlugin* plugin)
{
    auto it = pluginViews_.find(plugin);
    if (it == pluginViews_.end())
        return;

    QWidget* widget = it.value();
    ui_->stackedWidget_runningPlugins->removeWidget(widget);
    widget->setParent(nullptr);
    plugin->destroyView(widget);
}

}
