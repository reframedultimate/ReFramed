#pragma once

#include <QDialog>

namespace Ui {
    class ReplayEditorDialog;
}

namespace rfcommon {
    class Session;
}

namespace rfapp {

class ReplayEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ReplayEditorDialog(rfcommon::Session* session, QWidget* parent=nullptr);
    ~ReplayEditorDialog();

private slots:
    void onSaveClicked();

private:
    Ui::ReplayEditorDialog* ui_;
};

}
