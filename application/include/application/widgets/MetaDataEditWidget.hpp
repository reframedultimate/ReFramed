#pragma once

#include "rfcommon/MetaDataListener.hpp"
#include "rfcommon/Reference.hpp"
#include <QWidget>
#include <QVector>

class QToolButton;
class QParallelAnimationGroup;
class QScrollArea;
class QLabel;

namespace rfcommon {
    class MetaData;
}

namespace rfapp {

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
class MetaDataEditWidget
        : public QWidget
        , public rfcommon::MetaDataListener
{
    Q_OBJECT

public:
    explicit MetaDataEditWidget(QWidget* parent=nullptr);
    ~MetaDataEditWidget();

    virtual void setMetaData(rfcommon::MetaData* mdata);
    virtual void clearMetaData();

    void setTitle(const QString& title);
    QWidget* contentWidget();

    rfcommon::MetaData* metaData() { return mdata_; }

    /*!
     * \brief Derived classes should return a list of widgets that
     * should be ignored by the scroll wheel
     */
    virtual QVector<QWidget*> scrollIgnoreWidgets() = 0;

public slots:
    void setExpanded(bool expanded);
    void onToggleButtonClicked(bool checked);

protected:
    void updateSize();

private:
    rfcommon::Reference<rfcommon::MetaData> mdata_;
    QToolButton* toggleButton_;
    QLabel* title_;
    QParallelAnimationGroup* toggleAnimation_;
    QWidget* contentArea_;
    int animationDuration_;
    int collapsedHeight_;
};

}
