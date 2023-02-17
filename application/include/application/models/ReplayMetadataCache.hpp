#pragma once

#include "application/listeners/ReplayGroupListener.hpp"
#include "application/listeners/ReplayManagerListener.hpp"

#include "rfcommon/Costume.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/Round.hpp"
#include "rfcommon/SetFormat.hpp"
#include "rfcommon/ScoreCount.hpp"

#include <QString>
#include <QHash>

namespace rfcommon {
	class FilePathResolver;
	class MappingInfo;
	class Metadata;
}

namespace rfapp {

class ReplayManager;

class ReplayMetadataCache : public ReplayGroupListener
{
public:
	ReplayMetadataCache(ReplayManager* manager, rfcommon::FilePathResolver* pathResolver);
	~ReplayMetadataCache();

	void load();
	void save();

	struct Entry
	{
		//! Date and time of this replay
		QString date;
		QString time;

		//! Player names
		QString p1name;
		QString p2name;

		QString stage;
		QString round;
		QString format;
		QString score;
		QString game;

		//! FighterIDs
		rfcommon::FighterID p1fighterID;
		rfcommon::FighterID p2fighterID;

		//! Fighter costumes
		rfcommon::Costume p1costume;
		rfcommon::Costume p2costume;
	};

	const Entry* lookupFilename(const QString& filename);

private:
	const Entry* newEntry(const QString& filename, rfcommon::MappingInfo* map, rfcommon::Metadata* mdata);

private:
	void onReplayGroupFileAdded(ReplayGroup* group, const QString& fileName) override;
	void onReplayGroupFileRemoved(ReplayGroup* group, const QString& fileName) override;

private:
	ReplayManager* manager_;
	rfcommon::FilePathResolver* pathResolver_;
	QHash<QString, Entry> entries_;
};

}
