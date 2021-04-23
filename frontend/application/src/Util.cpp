#include "application/Util.hpp"
#include "uh/Recording.hpp"
#include "uh/PlayerState.hpp"
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
    return qHash(c.canonicalPath().constData(), seed);
}
qhash_result_t qHash(const QFileInfo& c, qhash_result_t seed) noexcept
{
    return qHash(c.absoluteFilePath().constData(), seed);
}

namespace uhapp {

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
QString composeFileName(const uh::Recording* recording)
{
    QString date = QDateTime::fromMSecsSinceEpoch(recording->timeStampStartedMs()).toString("yyyy-MM-dd");
    QStringList playerList;
    for (int i = 0; i < recording->playerCount(); ++i)
    {
        const std::string* fighterName = recording->mappingInfo().fighterID.map(
                    recording->playerFighterID(i));
        if (fighterName)
            playerList.append(QString::fromStdString(recording->playerName(i) + " (" + *fighterName + ")"));
        else
            playerList.append(QString::fromStdString(recording->playerName(i)));
    }
    QString players = playerList.join(" vs ");
    QString formatDesc = QString::fromStdString(recording->format().description());
    QString setNumber = QString::number(recording->setNumber());
    QString gameNumber = QString::number(recording->gameNumber());

    if (recording->setNumber() == 1)
        return date + " - " + formatDesc + " - " + players + " Game " + gameNumber + ".uhr";

    return date + " - " + formatDesc + " (" + setNumber + ") - " + players + " Game " + gameNumber + ".uhr";
}

}
