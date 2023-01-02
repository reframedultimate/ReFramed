#pragma once

#include "application/listeners/MetadataEditListener.hpp"

#include <QWidget>
#include <QVector>

class QToolButton;
class QParallelAnimationGroup;
class QScrollArea;
class QLabel;

namespace rfapp {

class IconLabel;
class MetadataEditModel;

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
class MetadataEditWidget
        : public QWidget
        , public MetadataEditListener
{
    Q_OBJECT

public:
    explicit MetadataEditWidget(MetadataEditModel* model, QWidget* parent=nullptr);
    ~MetadataEditWidget();

    void setTitle(const QIcon& icon, const QString& title);
    QWidget* contentWidget();

    /*!
     * \brief Derived classes should return a list of widgets that
     * should be ignored by the scroll wheel
     */
    virtual QVector<QWidget*> scrollIgnoreWidgets() = 0;

    bool isExpanded() const;

public slots:
    void setExpanded(bool expanded);
    void onToggleButtonClicked(bool checked);

protected:
    void updateSize();

    MetadataEditModel* model_;

private:
    QToolButton* toggleButton_;
    IconLabel* title_;
    QParallelAnimationGroup* toggleAnimation_;
    QWidget* contentArea_;
    int animationDuration_;
    int collapsedHeight_;
};

}
