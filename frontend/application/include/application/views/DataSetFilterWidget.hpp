#pragma once

#include <QWidget>

class QToolButton;
class QCheckBox;
class QParallelAnimationGroup;
class QScrollArea;

namespace uhapp {

class DataSetFilter;

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
    explicit DataSetFilterWidget(DataSetFilter* filter, QWidget* parent=nullptr);

    void setTitle(const QString& title);
    QWidget* contentWidget();

    DataSetFilter* filter() const;

signals:
    void enableFilter(bool);
    void moveFilterUp();
    void moveFilterDown();
    void removeFilterRequested();

public slots:
    void setExpanded(bool expanded);

protected:
    void updateSize();

private slots:
    void onToolButtonClicked(bool checked);

private:
    QToolButton* toggleButton_;
    QCheckBox* enableCheckbox_;
    QParallelAnimationGroup* toggleAnimation_;
    QScrollArea* contentArea_;
    int animationDuration_;
    int collapsedHeight_;
};

}

