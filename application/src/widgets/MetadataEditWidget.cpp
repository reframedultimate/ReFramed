#include "rfcommon/Profiler.hpp"
#include "application/models/MetadataEditModel.hpp"
#include "application/widgets/MetadataEditWidget.hpp"
#include "application/widgets/IconLabel.hpp"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QToolButton>
#include <QScrollArea>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QCheckBox>
#include <QPainter>
#include <QLabel>

namespace rfapp {

// ----------------------------------------------------------------------------
MetadataEditWidget::MetadataEditWidget(MetadataEditModel* model, QWidget* parent)
    : QWidget(parent)
    , model_(model)
    , toggleButton_(new QToolButton)
    , title_(new IconLabel(QSize(16, 16)))
    , toggleAnimation_(new QParallelAnimationGroup(this))
    , contentArea_(new QWidget)
    , animationDuration_(100)
{
    toggleButton_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toggleButton_->setArrowType(Qt::RightArrow);
    toggleButton_->setCheckable(true);
    toggleButton_->setAutoRaise(true);
    toggleButton_->setContentsMargins(0, 0, 0, 0);
    toggleButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    title_->setContentsMargins(0, 0, 0, 0);
    title_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

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

    // don't waste spaceIt's proba
    int column = 0;
    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->setVerticalSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(toggleButton_,  0, column++, 1, 1, Qt::AlignLeft);
    mainLayout->addWidget(title_, 0, column++, 1, 1, Qt::AlignLeft);
    mainLayout->setColumnStretch(column-1, 1); // title should use as much space as possible
    mainLayout->addWidget(headerLine, 1, 0, 1, column);
    mainLayout->addWidget(contentArea_, 2, 0, 1, column);
    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    /*
     * Store this for later, as it seems like it is impossible to get the
     * correct value in updateSize() after widgets have been added to the
     * contents area.
     */
    collapsedHeight_ = sizeHint().height();

    connect(toggleButton_, &QToolButton::clicked, this, &MetadataEditWidget::onToggleButtonClicked);

    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
MetadataEditWidget::~MetadataEditWidget()
{
    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void MetadataEditWidget::setTitle(const QIcon& icon, const QString& title)
{
    PROFILE(MetadataEditWidget, setTitle);

    title_->setIcon(icon);
    title_->setText(title);
}

// ----------------------------------------------------------------------------
QWidget* MetadataEditWidget::contentWidget()
{
    PROFILE(MetadataEditWidget, contentWidget);

    return contentArea_;
}

// ----------------------------------------------------------------------------
void MetadataEditWidget::setExpanded(bool expanded)
{
    PROFILE(MetadataEditWidget, setExpanded);

    toggleButton_->setChecked(expanded);
    onToggleButtonClicked(expanded);
}

// ----------------------------------------------------------------------------
void MetadataEditWidget::updateSize()
{
    PROFILE(MetadataEditWidget, updateSize);

    // http://stackoverflow.com/questions/13942616/qt-resize-window-after-widget-remove
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    QPropertyAnimation* anim;
    int contentHeight = contentArea_->sizeHint().height();

    // Update animation extents
    anim = static_cast<QPropertyAnimation*>(toggleAnimation_->animationAt(0));
    anim->setDuration(animationDuration_);
    anim->setStartValue(collapsedHeight_);
    anim->setEndValue(collapsedHeight_ + contentHeight);

    anim = static_cast<QPropertyAnimation*>(toggleAnimation_->animationAt(1));
    anim->setDuration(animationDuration_);
    anim->setStartValue(collapsedHeight_);
    anim->setEndValue(collapsedHeight_ + contentHeight);

    anim = static_cast<QPropertyAnimation*>(toggleAnimation_->animationAt(2));
    anim->setDuration(animationDuration_);
    anim->setStartValue(0);
    anim->setEndValue(contentHeight);

    // Updates the height of this widget and the contents widget
    if(toggleButton_->isChecked())
    {
        contentArea_->setMaximumHeight(contentHeight);
        setMinimumHeight(collapsedHeight_ + contentHeight);
        setMaximumHeight(collapsedHeight_ + contentHeight);
    }
}

// ----------------------------------------------------------------------------
void MetadataEditWidget::onToggleButtonClicked(bool checked)
{
    PROFILE(MetadataEditWidget, onToggleButtonClicked);

    toggleButton_->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
    toggleAnimation_->setDirection(checked ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    toggleAnimation_->start();
}

}
