#pragma once

#include <QLineEdit>

namespace rfapp {

class ReplaySearchBox : public QLineEdit
{
    Q_OBJECT
public:
    enum TermType
    {
        GENERIC
    };

    explicit ReplaySearchBox(QWidget* parent=nullptr);
    ~ReplaySearchBox();

signals:
    void searchTextChanged(int type, const QStringList& text);

private slots:
    void onTextChanged(const QString& text);
};

}
