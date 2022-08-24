#pragma once

#include <QDialog>

namespace Ui {
    class LicenseView;
}

class LicenseDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LicenseDialog(QWidget* parent = nullptr);
    ~LicenseDialog();

private slots:
    void onRadioButtonAgreeToggled(bool enable);
    void onPushButtonContinueReleased();
    void onPushButtonCancelReleased();

private:
    Ui::LicenseView* ui_;
};
