#include "application/widgets/ProgressDialog.hpp"
#include <QProgressBar>
#include <QLabel>
#include <QIcon>
#include <QVBoxLayout>
#include <QApplication>

namespace rfapp {

// ----------------------------------------------------------------------------
ProgressDialog::ProgressDialog(const QString& title, const QString& text, QWidget* parent)
    : QWidget(parent)
    , bar_(new QProgressBar)
    , info_(new QLabel)
{
    setWindowTitle(title);
    //setWindowFlags(Qt::WindowStaysOnTopHint);

    bar_->setMinimum(0);
    bar_->setMaximum(100);
    bar_->setValue(0);

    info_->setVisible(false);

    QLabel* label = new QLabel(text);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(info_);
    layout->addWidget(bar_);
    setLayout(layout);
}

// ----------------------------------------------------------------------------
ProgressDialog::~ProgressDialog()
{}

// ----------------------------------------------------------------------------
void ProgressDialog::setPercent(int percent, const QString& text)
{
    bar_->setValue(percent);

    if (text.length() > 0)
    {
        info_->setVisible(true);
        info_->setText(text);
    }

    qApp->processEvents();
}

}
