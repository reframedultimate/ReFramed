#pragma once

#include "uh/Reference.hpp"
#include "uh/DataSetFilter.hpp"  // required by MOC
#include <QWidget>

class QToolButton;
class QCheckBox;
class QParallelAnimationGroup;
class QScrollArea;

namespace uhapp {

/*!
 * @brief Collapsible widget. Code was copied from here and adapted:
 * http://stackoverflow.com/questions/32476006/how-to-make-an-expandable-collapsable-section-widget-in-qt
 *
 * Derived classes should place their custom widges into contentWidget() and
 * then call updateSize() when done. If you are using UI forms then do:
 * ```cpp
 * ui->setupUi(contentWidget()); // In constructor
 * ui->retranslateUi(contentWidget()); // When retranslating
 * ```
 *
 * You should also call ```setExpanded(true);``` once you've finished
 * constructing, so your widget is initially expanded.
 */
class DataSetFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DataSetFilterWidget(uh::DataSetFilter* filter, QWidget* parent=nullptr);

    void setTitle(const QString& title);
    QWidget* contentWidget();

    uh::DataSetFilter* filter() const;

signals:
    void enableFilter(DataSetFilterWidget*, bool);
    void invertFilter(DataSetFilterWidget*, bool);
    void moveFilterUp(DataSetFilterWidget*);
    void moveFilterDown(DataSetFilterWidget*);
    void removeFilterRequested(DataSetFilterWidget*);

public slots:
    void setExpanded(bool expanded);

protected:
    void updateSize();

private slots:
    void onToggleButtonClicked(bool checked);

private:
    uh::Reference<uh::DataSetFilter> filter_;
    QToolButton* toggleButton_;
    QCheckBox* enableCheckbox_;
    QCheckBox* notCheckbox_;
    QParallelAnimationGroup* toggleAnimation_;
    QScrollArea* contentArea_;
    int animationDuration_;
    int collapsedHeight_;
};

}

