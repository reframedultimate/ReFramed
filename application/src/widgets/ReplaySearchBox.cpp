#include "application/widgets/ReplaySearchBox.hpp"

#include <QDebug>

namespace rfapp {
    
// ----------------------------------------------------------------------------
ReplaySearchBox::ReplaySearchBox(QWidget* parent)
    : QLineEdit(parent)
{
    connect(this, &QLineEdit::textChanged, this, &ReplaySearchBox::onTextChanged);
}

// ----------------------------------------------------------------------------
ReplaySearchBox::~ReplaySearchBox()
{}

// ----------------------------------------------------------------------------
void ReplaySearchBox::onTextChanged(const QString& text)
{
    if (text.isEmpty())
    {
        emit searchTextChanged(GENERIC, {});
        return;
    }

    QStringList terms = text.split(QRegExp("[ ,]"));
    emit searchTextChanged(GENERIC, terms);
}

}
