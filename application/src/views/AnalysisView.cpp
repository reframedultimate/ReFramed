#include "application/ui_AnalysisInputView.h"
#include "application/views/AnalysisView.hpp"
#include "application/models/PluginManager.hpp"
#include "uh/AnalyzerPlugin.hpp"
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QMenu>
#include <QMouseEvent>
#include <QIcon>

namespace uhapp {

// ----------------------------------------------------------------------------
AnalysisView::AnalysisView(PluginManager* pluginManager, QWidget* parent)
    : QWidget(parent)
    , pluginManager_(pluginManager)
    , inputUi_(new Ui::AnalysisInputView)
    , tabWidget_(new QTabWidget)
    , addMenu_(new QMenu(this))
{
    setLayout(new QVBoxLayout);
    layout()->addWidget(tabWidget_);

    for (const auto& name : pluginManager_->availableFactoryNames(UHPluginType::ANALYZER))
        addMenu_->addAction(name);

    QWidget* inputSettingsTab = new QWidget;
    inputUi_->setupUi(inputSettingsTab);

    tabWidget_->addTab(inputSettingsTab, "Settings");
    tabWidget_->addTab(new QWidget, "+");

    connect(tabWidget_, &QTabWidget::tabBarClicked,
            this, &AnalysisView::onTabBarClicked);
    connect(tabWidget_, &QTabWidget::currentChanged,
            this, &AnalysisView::onTabIndexChanged);
}

// ----------------------------------------------------------------------------
AnalysisView::~AnalysisView()
{
    for (auto it = loadedPlugins_.begin(); it != loadedPlugins_.end(); ++it)
    {
        const QString& name = it.key();
        uh::AnalyzerPlugin* model = it.value().model;
        QWidget* view = it.value().view;

        int tabIndex = tabWidget_->indexOf(view);
        tabWidget_->removeTab(tabIndex);

        model->destroyView(view);
        pluginManager_->destroyModel(name, model);
    }

    delete inputUi_;
}

// ----------------------------------------------------------------------------
void AnalysisView::onTabBarClicked(int index)
{
    if (index != tabWidget_->count() - 1)
        return;

    QAction* action = addMenu_->exec(QCursor::pos());
    if (action == nullptr)
        return;

    int insertIdx = tabWidget_->count() - 1;

    const QString& name = action->text();
    uh::AnalyzerPlugin* model = pluginManager_->createAnalyzerModel(name);
    if (model == nullptr)
        return;

    QWidget* view = model->createView();
    if (view == nullptr)
    {
        pluginManager_->destroyModel(name, model);
        return;
    }

    tabWidget_->insertTab(
        insertIdx,
        view,
        action->text()
    );
    loadedPlugins_.insert(name, {model, view});

    QToolButton* closeButton = new QToolButton();
    closeButton->setIcon(style()->standardIcon(QStyle::SP_DockWidgetCloseButton));
    tabWidget_->tabBar()->setTabButton(insertIdx, QTabBar::RightSide, closeButton);

    connect(closeButton, &QToolButton::released, [this, model, view](){
        closeTab(view);
    });
}

// ----------------------------------------------------------------------------
void AnalysisView::onTabIndexChanged(int index)
{
    // Prevent the "+" tab from ever being selected
    if (index == tabWidget_->count() - 1)
        tabWidget_->setCurrentIndex(index - 1);
}

// ----------------------------------------------------------------------------
void AnalysisView::closeTab(QWidget* widget)
{
    for (auto it = loadedPlugins_.begin(); it != loadedPlugins_.end(); ++it)
    {
        const QString& name = it.key();
        uh::AnalyzerPlugin* model = it.value().model;
        QWidget* view = it.value().view;

        if (view == widget)
        {
            int tabIndex = tabWidget_->indexOf(widget);
            tabWidget_->removeTab(tabIndex);

            model->destroyView(view);
            pluginManager_->destroyModel(name, model);
            loadedPlugins_.erase(it);
            break;
        }
    }
}

}
