#include "application/views/CategoryTabsView.hpp"

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
CategoryTabsView::CategoryTabsView(QWidget* parent)
    : QTabWidget(parent)
{

    setTabPosition(QTabWidget::West);
    //tabBar()->setStyle(new CustomTabStyle);
    tabBar()->setObjectName("categoryTab");

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
        "   height: 140px;\n"
        "   font-size: 14pt;\n"
        "}\n"
    );
}

// ----------------------------------------------------------------------------
CategoryTabsView::~CategoryTabsView()
{}

}
