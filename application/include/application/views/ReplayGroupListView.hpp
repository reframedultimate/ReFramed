#pragma once

#include <QListWidget>

namespace rfapp {

class ReplayGroupListView : public QListWidget
{
public:
    explicit ReplayGroupListView(QWidget* parent=nullptr);
    ~ReplayGroupListView();

private:
};

}
