#include "application/ui_DataSetFilterView.h"
#include "application/views/DataSetFilterView.hpp"
#include "application/views/DataSetFilterWidget_Matchup.hpp"
#include "application/views/DataSetFilterWidget_Player.hpp"
#include "application/views/DataSetFilterWidget_Stage.hpp"

#include <QMenu>

#define FILTER_LIST \
    X(Matchup) \
    X(Player) \
    X(Stage)

namespace uhapp {

// ----------------------------------------------------------------------------
DataSetFilterView::DataSetFilterView(QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::DataSetFilterView)
    , filterWidgetsLayout_(new QVBoxLayout)
{
    ui_->setupUi(this);

    /*
     * The QAction objects responsible for adding a new filters are created in
     * QtDesigner and are available through the ui. Create a menu and add each
     * action to the menu so the user can add modifiers.
     */
    QMenu* menu = new QMenu(this);
#define X(filter) menu->addAction(ui_->action##filter);
    FILTER_LIST
#undef X
    ui_->toolButton_addFilter->setMenu(menu);

    // Filter widgets should align to the top, not in the middle
    ui_->scrollAreaWidgetContents_filters->setLayout(filterWidgetsLayout_);
    filterWidgetsLayout_->setAlignment(Qt::AlignTop);

    connect(ui_->toolButton_addFilter, &QToolButton::triggered,
            this, &DataSetFilterView::onToolButtonAddFilterTriggered);
}

// ----------------------------------------------------------------------------
DataSetFilterView::~DataSetFilterView()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
bool DataSetFilterView::eventFilter(QObject* target, QEvent* e)
{
    // Stops the scroll wheel from interfering with the elements in each
    // modifier widget
    if(e->type() == QEvent::Wheel)
        return true;
    return false;
}


// ----------------------------------------------------------------------------
void DataSetFilterView::onToolButtonAddFilterTriggered(QAction* action)
{
    if (0) {}
#define X(filter) \
    else if (action == ui_->action##filter) \
        addNewFilterWidget(new DataSetFilterWidget_##filter);
    FILTER_LIST
#undef X
}

// ----------------------------------------------------------------------------
void DataSetFilterView::addNewFilterWidget(DataSetFilterWidget* widget)
{
    filterWidgetsLayout_->addWidget(widget);
    recursivelyInstallEventFilter(widget);
}

// ----------------------------------------------------------------------------
void DataSetFilterView::recursivelyInstallEventFilter(QObject* obj)
{
    const QObjectList& children = obj->children();
    for(QObjectList::const_iterator child = children.begin(); child != children.end(); ++child)
        recursivelyInstallEventFilter(*child);

    if (obj->isWidgetType())
        obj->installEventFilter(this);
}

}
