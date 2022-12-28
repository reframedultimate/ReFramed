#pragma once

#include "application/widgets/MetadataEditWidget.hpp"

class QVBoxLayout;
class QLineEdit;

namespace rfapp {

class MetadataEditWidget_Tournament : public MetadataEditWidget
{
    Q_OBJECT

public:
    explicit MetadataEditWidget_Tournament(MetadataEditModel* model, QWidget* parent=nullptr);
    ~MetadataEditWidget_Tournament();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private slots:
    void onAddTOReleased();
    void onAddSponsorReleased();

private:
    void addTOUI(const QString& name, const QString& social, const QString& pronouns);
    void addSponsorUI(const QString& name, const QString& website);

private:
    void onAdoptMetadata(const MappingInfoList& map, const MetadataList& mdata) override;
    void onOverwriteMetadata(const MappingInfoList& map, const MetadataList& mdata) override;
    void onMetadataCleared(const MappingInfoList& map, const MetadataList& mdata) override;
    void onNextGameStarted() override;
    void onBracketTypeChangedUI(rfcommon::BracketType bracketType) override;

    void onMetadataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) override;
    void onMetadataTournamentDetailsChanged() override;
    void onMetadataEventDetailsChanged() override;
    void onMetadataCommentatorsChanged() override;
    void onMetadataGameDetailsChanged() override;
    void onMetadataPlayerDetailsChanged() override;
    void onMetadataWinnerChanged(int winnerPlayerIdx) override;
    void onMetadataTrainingSessionNumberChanged(rfcommon::SessionNumber number) override;

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
