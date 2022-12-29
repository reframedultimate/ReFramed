#pragma once

#include <QWidget>
#include <QIcon>

class QLabel;

namespace rfapp {

class IconLabel : public QWidget
{
public:
    explicit IconLabel(const QSize& size, QWidget* parent=nullptr);
    ~IconLabel();

    void setText(const QString& text);
    void setIcon(const QIcon& icon);

protected:
    void changeEvent(QEvent* e) override;

private:
    QSize size_;
    QIcon icon_;
    QLabel* labelIcon_;
    QLabel* labelText_;
};

}
