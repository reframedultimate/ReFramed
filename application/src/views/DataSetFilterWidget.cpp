#include "application/views/DataSetFilterWidget.hpp"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QToolButton>
#include <QCheckBox>
#include <QScrollArea>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QCheckBox>
#include <QPainter>

namespace rfapp {

// ----------------------------------------------------------------------------
DataSetFilterWidget::DataSetFilterWidget(rfcommon::DataSetFilter* filter, QWidget* parent)
    : QWidget(parent)
    , filter_(filter)
    , toggleButton_(new QToolButton)
    , enableCheckbox_(new QCheckBox)
    , notCheckbox_(new QCheckBox("Not"))
    , toggleAnimation_(new QParallelAnimationGroup(this))
    , contentArea_(new QScrollArea)
    , animationDuration_(100)
{
    toggleButton_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toggleButton_->setArrowType(Qt::RightArrow);
    toggleButton_->setCheckable(true);
    toggleButton_->setAutoRaise(true);
    toggleButton_->setContentsMargins(0, 0, 0, 0);
    toggleButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    enableCheckbox_->setChecked(true);
    enableCheckbox_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    enableCheckbox_->setContentsMargins(0, 0, 0, 0);
    enableCheckbox_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    notCheckbox_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    notCheckbox_->setContentsMargins(0, 0, 0, 0);
    notCheckbox_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    QToolButton* moveUpButton = new QToolButton;
    moveUpButton->setArrowType(Qt::UpArrow);
    moveUpButton->setFixedWidth(30);
    moveUpButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    moveUpButton->setContentsMargins(0, 0, 0, 0);

    QToolButton* moveDownButton = new QToolButton;
    moveDownButton->setArrowType(Qt::DownArrow);
    moveDownButton->setFixedWidth(30);
    moveDownButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    moveDownButton->setContentsMargins(0, 0, 0, 0);

    QToolButton* closeButton = new QToolButton;
    closeButton->setAutoRaise(true);
    closeButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    closeButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    closeButton->setContentsMargins(0, 0, 0, 0);

    QFrame* headerLine = new QFrame;
    headerLine->setFrameShape(QFrame::HLine);
    headerLine->setFrameShadow(QFrame::Sunken);
    headerLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    contentArea_->setStyleSheet("QToolButton { border: none; }");
    contentArea_->setStyleSheet("QScrollArea { background-color: white; border: none; }");
    contentArea_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // start out collapsed
    contentArea_->setMaximumHeight(0);
    contentArea_->setMinimumHeight(0);

    // let the entire widget grow and shrink with its content
    toggleAnimation_->addAnimation(new QPropertyAnimation(this, "minimumHeight"));
    toggleAnimation_->addAnimation(new QPropertyAnimation(this, "maximumHeight"));
    toggleAnimation_->addAnimation(new QPropertyAnimation(contentArea_, "maximumHeight"));

    // don't waste space
    int column = 0;
    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->setVerticalSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(toggleButton_,  0, column++, 1, 1, Qt::AlignLeft);
    mainLayout->addWidget(enableCheckbox_,    0, column++, 1, 1, Qt::AlignLeft);
    mainLayout->addWidget(notCheckbox_,    0, column, 1, 1, Qt::AlignRight);
    mainLayout->setColumnStretch(column++, 1); // title should use as much space as possible
    mainLayout->addWidget(moveUpButton,   0, column++);
    mainLayout->addWidget(moveDownButton, 0, column++);
    mainLayout->addWidget(closeButton,    0, column++);
    mainLayout->addWidget(headerLine,     1, 0, 1, column);
    mainLayout->addWidget(contentArea_,   2, 0, 1, column);
    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    /*
     * Store this for later, as it seems like it is impossible to get the
     * correct value in updateSize() after widgets have been added to the
     * contents area.
     */
    collapsedHeight_ = sizeHint().height();

    connect(toggleButton_, &QToolButton::clicked, this, &DataSetFilterWidget::onToggleButtonClicked);
    connect(enableCheckbox_, &QCheckBox::toggled, [this](bool checked){ emit enableFilter(this, checked); });
    connect(notCheckbox_, &QCheckBox::toggled, [this](bool checked){ emit invertFilter(this, checked); });
    connect(moveUpButton, &QToolButton::released, [this](){ emit moveFilterUp(this); });
    connect(moveDownButton, &QToolButton::released, [this](){ emit moveFilterDown(this); });
    connect(closeButton, &QToolButton::released, [this](){ emit removeFilterRequested(this); });
}

// ----------------------------------------------------------------------------
void DataSetFilterWidget::setTitle(const QString& title)
{
    enableCheckbox_->setText(title);
}

// ----------------------------------------------------------------------------
QWidget* DataSetFilterWidget::contentWidget()
{
    return contentArea_;
}

// ----------------------------------------------------------------------------
rfcommon::DataSetFilter* DataSetFilterWidget::filter() const
{
    return filter_;
}

// ----------------------------------------------------------------------------
void DataSetFilterWidget::setExpanded(bool expanded)
{
    toggleButton_->setChecked(expanded);
    onToggleButtonClicked(expanded);
}

// ----------------------------------------------------------------------------
void DataSetFilterWidget::updateSize()
{
    // http://stackoverflow.com/questions/13942616/qt-resize-window-after-widget-remove
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    // Update animation extents
    int contentHeight = contentArea_->layout()->sizeHint().height();
    for(int i = 0; i < toggleAnimation_->animationCount() - 1; ++i)
    {
        QPropertyAnimation* spoilerAnimation = static_cast<QPropertyAnimation*>(toggleAnimation_->animationAt(i));
        spoilerAnimation->setDuration(animationDuration_);
        spoilerAnimation->setStartValue(collapsedHeight_);
        spoilerAnimation->setEndValue(collapsedHeight_ + contentHeight);
    }
    QPropertyAnimation* contentAnimation = static_cast<QPropertyAnimation*>(
        toggleAnimation_->animationAt(toggleAnimation_->animationCount() - 1));
    contentAnimation->setDuration(animationDuration_);
    contentAnimation->setStartValue(0);
    contentAnimation->setEndValue(contentHeight);

    // Updates the height of this widget and the contents widget
    if(toggleButton_->isChecked())
    {
        contentArea_->setMaximumHeight(contentHeight);
        setMinimumHeight(collapsedHeight_ + contentHeight);
        setMaximumHeight(collapsedHeight_ + contentHeight);
    }
}

// ----------------------------------------------------------------------------
void DataSetFilterWidget::onToggleButtonClicked(bool checked)
{
    toggleButton_->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
    toggleAnimation_->setDirection(checked ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    toggleAnimation_->start();
}

}
