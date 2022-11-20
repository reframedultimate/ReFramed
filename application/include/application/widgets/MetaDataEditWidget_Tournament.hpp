#pragma once

#include "application/widgets/MetaDataEditWidget.hpp"

class QVBoxLayout;
class QLineEdit;

namespace rfapp {

class MetaDataEditWidget_Tournament : public MetaDataEditWidget
{
    Q_OBJECT

public:
    explicit MetaDataEditWidget_Tournament(MetaDataEditModel* model, QWidget* parent=nullptr);
    ~MetaDataEditWidget_Tournament();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private slots:
    void onAddTOReleased();
    void onAddSponsorReleased();

private:
    void addTOUI(const QString& name, const QString& social, const QString& pronouns);
    void addSponsorUI(const QString& name, const QString& website);

private:
    void onAdoptMetaData(const MappingInfoList& map, const MetaDataList& mdata) override;
    void onOverwriteMetaData(const MappingInfoList& map, const MetaDataList& mdata) override;
    void onMetaDataCleared(const MappingInfoList& map, const MetaDataList& mdata) override;
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
    struct OrganizerWidgets
    {
        QLineEdit* name;
        QLineEdit* social;
        QLineEdit* pronouns;
    };

    struct SponsorWidgets
    {
        QLineEdit* name;
        QLineEdit* website;
    };

    QLineEdit* lineEdit_name_;
    QLineEdit* lineEdit_website_;
    QVBoxLayout* layout_TOs_;
    QVBoxLayout* layout_sponsors_;

    QVector<OrganizerWidgets> organizerWidgets_;
    QVector<SponsorWidgets> sponsorWidgets_;

    bool ignoreSelf_ = false;
};

}
