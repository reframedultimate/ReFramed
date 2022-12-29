#include "application/widgets/IconLabel.hpp"

#include <QEvent>
#include <QLabel>
#include <QIcon>
#include <QHBoxLayout>

namespace rfapp {

// ----------------------------------------------------------------------------
IconLabel::IconLabel(const QSize& size, QWidget* parent)
    : QWidget(parent)
    , size_(size)
    , labelIcon_(new QLabel)
    , labelText_(new QLabel)
{
    setLayout(new QHBoxLayout);
    layout()->addWidget(labelIcon_);
    layout()->addWidget(labelText_);
}

// ----------------------------------------------------------------------------
IconLabel::~IconLabel()
{}

// ----------------------------------------------------------------------------
void IconLabel::setText(const QString& text)
{
   labelText_->setText(text);
}

// ----------------------------------------------------------------------------
void IconLabel::setIcon(const QIcon& icon)
{
    labelIcon_->setPixmap(icon.pixmap(size_));
    icon_ = icon;
}

// ----------------------------------------------------------------------------
void IconLabel::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::StyleChange)
    {
        labelIcon_->setPixmap(icon_.pixmap(size_));
    }

    QWidget::changeEvent(e);
}

}
