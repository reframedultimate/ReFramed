#include "rfcommon/Profiler.hpp"
#include "application/widgets/ReplaySearchBox.hpp"

#include <QRegularExpression>

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
    PROFILE(ReplaySearchBox, onTextChanged);

    if (text.isEmpty())
    {
        emit searchTextChanged(GENERIC, {});
        return;
    }

    QStringList terms = text.split(QRegularExpression("[ ,]"));
    emit searchTextChanged(GENERIC, terms);
}

}
