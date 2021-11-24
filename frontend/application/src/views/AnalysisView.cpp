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

    for (const auto& name : pluginManager_->availableNames(UHPluginType::ANALYZER))
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
    for (const auto& it : loadedPlugins_)
    {
        int tabIndex = tabWidget_->indexOf(it.widget);
        tabWidget_->removeTab(tabIndex);

        it.widget->setParent(nullptr);
        it.plugin->destroyView(it.widget);

        pluginManager_->destroy(it.plugin);
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

    uh::AnalyzerPlugin* plugin = pluginManager_->createAnalyzer(action->text().toStdString().c_str());
    QWidget* pluginWidget = plugin->createView();
    tabWidget_->insertTab(
        insertIdx,
        pluginWidget,
        action->text()
    );
    loadedPlugins_.push(PluginAndWidget{plugin, pluginWidget});

    QToolButton* closeButton = new QToolButton();
    closeButton->setIcon(style()->standardIcon(QStyle::SP_DockWidgetCloseButton));
    tabWidget_->tabBar()->setTabButton(insertIdx, QTabBar::RightSide, closeButton);

    connect(closeButton, &QToolButton::released, [this, pluginWidget](){
        closeTab(pluginWidget);
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
        if (it->widget == widget)
        {
            it->widget->setParent(nullptr);
            it->plugin->destroyView(it->widget);

            int tabIndex = tabWidget_->indexOf(it->widget);
            tabWidget_->removeTab(tabIndex);

            pluginManager_->destroy(it->plugin);
            loadedPlugins_.erase(it);
            break;
        }
}

}
