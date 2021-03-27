#pragma once

#include <QObject>

namespace uh {

class Recorder : public QObject
{
    Q_OBJECT
public:
    explicit Recorder(QObject* parent=nullptr);

public slots:

};

}
