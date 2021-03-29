#pragma once

#include "uh/models/CategoryType.hpp"
#include <QTreeWidget>

namespace uh {

class CategoryView : public QTreeWidget
{
    Q_OBJECT
public:
    explicit CategoryView(QWidget* parent=nullptr);

signals:
    /*!
     * \brief When the user clicks on a top-level item. The main window will
     * want to switch the current widget in response to a category change.
     */
    void categoryChanged(CategoryType category);

private slots:
    void onTreeWidgetCategoriesCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private:
    QTreeWidgetItem* analysisCategoryItem_;
    QTreeWidgetItem* recordingGroupsCategoryItem_;
    QTreeWidgetItem* recordingSourcesCategoryItem_;
    QTreeWidgetItem* videoSourcesCategoryItem_;
    QTreeWidgetItem* activeRecordingCategoryItem_;
};

}
