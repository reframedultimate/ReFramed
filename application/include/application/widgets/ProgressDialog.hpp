#pragma once

#include <QWidget>

class QLabel;
class QProgressBar;

namespace rfapp {

class ProgressDialog : public QWidget
{
    Q_OBJECT

public:
    ProgressDialog(const QString& title, const QString& text);
    ~ProgressDialog();

public slots:
    void setPercent(int percent, const QString& text="");

private:
    QProgressBar* bar_;
    QLabel* info_;
};

}
