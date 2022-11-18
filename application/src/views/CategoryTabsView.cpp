#include "application/views/ActiveSessionView.hpp"
#include "application/views/CategoryTabsView.hpp"
#include "application/views/ReplayManagerView.hpp"

#include <QProxyStyle>
#include <QStyleOptionTab>

namespace rfapp {

class CustomTabStyle : public QProxyStyle
{
public:
    QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
    {
        QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        if (type == QStyle::CT_TabBarTab)
            s.transpose();
        return s;
    }

    void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
    {
        if (element == CE_TabBarTabLabel)
        {
            if (const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option))
            {
                QStyleOptionTab opt(*tab);
                opt.shape = QTabBar::RoundedNorth;
                QProxyStyle::drawControl(element, &opt, painter, widget);
                return;
            }
        }
        QProxyStyle::drawControl(element, option, painter, widget);
    }
};

// ----------------------------------------------------------------------------
CategoryTabsView::CategoryTabsView(
        ReplayManager* replayManager,
        PluginManager* pluginManager,
        ActiveSessionManager* activeSessionManager,
        UserMotionLabelsManager* userMotionLabelsManager,
        rfcommon::Hash40Strings* hash40Strings,
        QWidget* parent)
    : QTabWidget(parent)
    , replayManagerView_(new ReplayManagerView(replayManager, pluginManager, userMotionLabelsManager, hash40Strings))
    , activeSessionView_(new ActiveSessionView(activeSessionManager, pluginManager))
{
    setTabPosition(QTabWidget::West);
    //tabBar()->setStyle(new CustomTabStyle);
    tabBar()->setObjectName("categoryTab");

    addTab(activeSessionView_, "Session");
    addTab(replayManagerView_, "Replays");
    addTab(new QWidget, "Compare");
    addTab(new QWidget, "Marketplace");
    setCurrentIndex(1);

    setStyleSheet(
                /*
        "QTabBar::tab {\n"
            //"background: #f9b233;\n"
            //"color: white;\n"
        "}\n"

        "QTabBar::tab:selected {\n"
            "background: #f9b233;\n"
            //"background: lightgray;\n"
         "}\n"*/
        "QTabBar#categoryTab::tab {\n"
        "   width: 50px;\n"
        "   height: 160px;\n"
        "   font-size: 12pt;\n"
        "}\n"
    );

    connect(this, &QTabWidget::tabBarClicked, this, &CategoryTabsView::onTabBarClicked);
}

// ----------------------------------------------------------------------------
CategoryTabsView::~CategoryTabsView()
{}

// ----------------------------------------------------------------------------
void CategoryTabsView::onTabBarClicked(int index)
{
    if (index != currentIndex())
        return;

    if (index == 0)
        activeSessionView_->toggleSideBar();
    else if (index == 1)
        replayManagerView_->toggleSideBar();
}

}
