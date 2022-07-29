#include "application/Util.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/SessionMetaData.hpp"
#include "rfcommon/MappingInfo.hpp"
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
QString composeFileName(const rfcommon::Session* session)
{
    using namespace rfcommon;
    const SessionMetaData* meta = session->metaData();
    const MappingInfo* map = session->mappingInfo();

    const uint64_t stamp = meta->timeStampStarted().millisSinceEpoch();
    QString date = QDateTime::fromMSecsSinceEpoch(stamp).toString("yyyy-MM-dd");

    QStringList playerList;
    for (int i = 0; i < meta->fighterCount(); ++i)
    {
        const char* fighterName = map->fighterID.toName(meta->fighterID(i));
        if (fighterName)
            playerList.append((meta->name(i) + " (" + fighterName + ")").cStr());
        else
            playerList.append(meta->name(i).cStr());
    }
    QString players = playerList.join(" vs ");

    if (meta->type() == SessionMetaData::GAME)
    {
        const GameSessionMetaData* gameMeta = static_cast<const GameSessionMetaData*>(meta);
        QString formatDesc = gameMeta->setFormat().description().cStr();
        QString setNumber = QString::number(gameMeta->setNumber().value());
        QString gameNumber = QString::number(gameMeta->gameNumber().value());

        if (gameMeta->setNumber() == 1)
            return date + " - " + formatDesc + " - " + players + " Game " + gameNumber + ".rfr";
        return date + " - " + formatDesc + " (" + setNumber + ") - " + players + " Game " + gameNumber + ".rfr";
    }
    else
    {
        assert(meta->type() == SessionMetaData::TRAINING);
        const TrainingSessionMetaData* trainingMeta = static_cast<const TrainingSessionMetaData*>(meta);
        QString sessionNumber = QString::number(trainingMeta->sessionNumber().value());

        return date + " - Training - " + players + " Session " + sessionNumber + ".rfr";
    }
}

}
