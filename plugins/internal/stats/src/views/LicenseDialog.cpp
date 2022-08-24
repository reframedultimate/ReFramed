#include "ui_LicenseView.h"
#include "stats/config.hpp"
#include "stats/views/LicenseDialog.hpp"

// ----------------------------------------------------------------------------
LicenseDialog::LicenseDialog(QWidget* parent)
    : QDialog(parent)
    , ui_(new Ui::LicenseView)
{
    ui_->setupUi(this);

#if defined(STATS_ENABLE_LICENSING)
    ui_->textBrowser_license->setText(
        "This version of the Statistics Plugin is intended for use only at \"" STATS_LICENSED_TO "\". Do not share this version of the Statistics Plugin without permission from Vye (Vye#0547) or TheComet (TheComet#5387).\n"
        "\n"
        "Because a lot of features are still being worked on, the Statistics Plugin is not to be publicly released in its current form.");
#endif

    setWindowTitle("Statistics Plugin License Agreement");
    setModal(true);

    connect(ui_->radioButton_agree, &QRadioButton::toggled, this, &LicenseDialog::onRadioButtonAgreeToggled);
    connect(ui_->pushButton_cancel, &QPushButton::released, this, &LicenseDialog::onPushButtonCancelReleased);
    connect(ui_->pushButton_continue, &QPushButton::released, this, &LicenseDialog::onPushButtonContinueReleased);
}

// ----------------------------------------------------------------------------
LicenseDialog::~LicenseDialog()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void LicenseDialog::onRadioButtonAgreeToggled(bool enable)
{
    ui_->pushButton_continue->setEnabled(enable);
}

// ----------------------------------------------------------------------------
void LicenseDialog::onPushButtonContinueReleased()
{
    if (ui_->radioButton_agree->isChecked())
        accept();
}

// ----------------------------------------------------------------------------
void LicenseDialog::onPushButtonCancelReleased()
{
    reject();
}
