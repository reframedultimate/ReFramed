#pragma once

#include <QSplitter>

namespace rfapp {

class CollapsibleSplitter : public QSplitter
{
public:
    CollapsibleSplitter(Qt::Orientation orientation);
    ~CollapsibleSplitter();

    void toggleCollapse();

private:
    int store_ = 0;
};

}
