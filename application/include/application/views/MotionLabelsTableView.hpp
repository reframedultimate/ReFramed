#pragma once

#include <QTableView>

namespace rfapp {

class MotionLabelsTableView : public QTableView
{
public:
    MotionLabelsTableView(QWidget* parent=nullptr);

    void keyPressEvent(QKeyEvent* e) override;
    void dropEvent(QDropEvent* e) override;
};

}
