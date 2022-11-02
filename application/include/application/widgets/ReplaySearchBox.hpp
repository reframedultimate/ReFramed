#pragma once

#include <QLineEdit>

namespace rfapp {

class ReplaySearchBox : public QLineEdit
{
    Q_OBJECT
public:
    explicit ReplaySearchBox(QWidget* parent=nullptr);
    ~ReplaySearchBox();
};

}
