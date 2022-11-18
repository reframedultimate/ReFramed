#pragma once

#include "application/widgets/MetaDataEditWidget.hpp"

class QVBoxLayout;
class QLineEdit;

namespace rfapp {

class MetaDataEditWidget_Commentators : public MetaDataEditWidget
{
    Q_OBJECT

public:
    explicit MetaDataEditWidget_Commentators(MetaDataEditModel* model, QWidget* parent=nullptr);
    ~MetaDataEditWidget_Commentators();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private:
    void addCommentatorUI(const char* name, const char* social, const char* pronouns);

private slots:
    void onAddCommentatorReleased();

private:
    void onAdoptMetaData(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata) override;
    void onOverwriteMetaData(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata) override;
    void onMetaDataCleared(rfcommon::MappingInfo* map, rfcommon::MetaData* mdata) override;
    void onBracketTypeChangedUI(rfcommon::BracketType bracketType) override;

    void onMetaDataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) override;
    void onMetaDataTournamentDetailsChanged() override;
    void onMetaDataEventDetailsChanged() override;
    void onMetaDataCommentatorsChanged() override;
    void onMetaDataGameDetailsChanged() override;
    void onMetaDataPlayerDetailsChanged() override;
    void onMetaDataWinnerChanged(int winnerPlayerIdx) override;
    void onMetaDataTrainingSessionNumberChanged(rfcommon::SessionNumber number) override;

private:
    struct CommentatorWidgets
    {
        QLineEdit* name;
        QLineEdit* social;
        QLineEdit* pronouns;
    };

    QVBoxLayout* commentatorsLayout_;
    QVector<CommentatorWidgets> commentatorWidgets_;
    bool ignoreSelf_ = false;
};

}
