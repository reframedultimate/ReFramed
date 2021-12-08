#include "application/Util.hpp"
#include "rfcommon/RunningGameSession.hpp"
#include "rfcommon/PlayerState.hpp"
#include <QLayout>
#include <QLayoutItem>
#include <QWidget>
#include <QStackedWidget>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <cstring>

// ----------------------------------------------------------------------------
qhash_result_t qHash(const QDir& c, qhash_result_t seed) noexcept
{
    return qHash(c.canonicalPath().toUtf8(), seed);
}
qhash_result_t qHash(const QFileInfo& c, qhash_result_t seed) noexcept
{
    return qHash(c.absoluteFilePath().toUtf8(), seed);
}

namespace rfapp {

// ----------------------------------------------------------------------------
void clearLayout(QLayout* layout)
{
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr)
    {
        if (item->layout() != nullptr)
            item->layout()->deleteLater();
        if (item->widget() != nullptr)
            item->widget()->deleteLater();
    }
}

// ----------------------------------------------------------------------------
void clearStackedWidget(QStackedWidget* sw)
{
    while (sw->count())
    {
        QWidget* widget = sw->widget(0);
        sw->removeWidget(widget);
        widget->deleteLater();
    }
}

// ----------------------------------------------------------------------------
QString composeFileName(const rfcommon::GameSession* session)
{
    QString date = QDateTime::fromMSecsSinceEpoch(session->timeStampStartedMs()).toString("yyyy-MM-dd");
    QStringList playerList;
    for (int i = 0; i < session->playerCount(); ++i)
    {
        const rfcommon::String* fighterName = session->mappingInfo().fighterID.map(
                    session->playerFighterID(i));
        if (fighterName)
            playerList.append((session->playerName(i) + " (" + *fighterName + ")").cStr());
        else
            playerList.append(session->playerName(i).cStr());
    }
    QString players = playerList.join(" vs ");
    QString formatDesc = session->format().description().cStr();
    QString setNumber = QString::number(session->setNumber());
    QString gameNumber = QString::number(session->gameNumber());

    if (session->setNumber() == 1)
        return date + " - " + formatDesc + " - " + players + " Game " + gameNumber + ".rfr";

    return date + " - " + formatDesc + " (" + setNumber + ") - " + players + " Game " + gameNumber + ".rfr";
}

}
