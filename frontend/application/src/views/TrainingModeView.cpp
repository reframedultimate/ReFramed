#include "application/views/TrainingModeView.hpp"
#include "application/models/PluginManager.hpp"
#include "application/ui_TrainingModeView.h"
#include "uh/PluginInterface.hpp"
#include "uh/TrainingModePlugin.hpp"

namespace uhapp {

// ----------------------------------------------------------------------------
TrainingModeView::TrainingModeView(TrainingMode* training, PluginManager* pluginManager, QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::TrainingModeView)
    , training_(training)
    , pluginManager_(pluginManager)
{
    ui_->setupUi(this);

    ui_->pushButton_launch->setEnabled(false);

    for (const auto& name : pluginManager_->availableNames(UHPluginType::TRAINING_MODE))
        ui_->listWidget_plugins->addItem(name);

    connect(ui_->listWidget_plugins, &QListWidget::currentTextChanged, this, &TrainingModeView::currentTextChanged);
    connect(ui_->pushButton_launch, &QPushButton::released, this, &TrainingModeView::launchPressed);
}

// ----------------------------------------------------------------------------
TrainingModeView::~TrainingModeView()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void TrainingModeView::currentTextChanged(const QString& text)
{
    UHPluginInfo* info = pluginManager_->getInfo(text);
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
    uh::TrainingModePlugin* plugin;
    QListWidgetItem* item = ui_->listWidget_plugins->currentItem();
    if (item == nullptr)
        goto fail;

    plugin = pluginManager_->createTrainingMode(item->text());
    if (plugin == nullptr)
        goto fail;

    pluginManager_->destroy(plugin);

    return;

    fail : ui_->pushButton_launch->setEnabled(false);
}

}
