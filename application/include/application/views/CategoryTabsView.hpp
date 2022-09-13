#pragma once

#include <QTabWidget>

namespace rfapp {

class CategoryTabsView : public QTabWidget
{
    Q_OBJECT
public:
    explicit CategoryTabsView(QWidget* parent=nullptr);
    ~CategoryTabsView();

private:

};

}
