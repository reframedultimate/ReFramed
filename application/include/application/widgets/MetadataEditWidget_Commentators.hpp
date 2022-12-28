#pragma once

#include "application/widgets/MetadataEditWidget.hpp"

class QVBoxLayout;
class QLineEdit;

namespace rfapp {

class MetadataEditWidget_Commentators : public MetadataEditWidget
{
    Q_OBJECT

public:
    explicit MetadataEditWidget_Commentators(MetadataEditModel* model, QWidget* parent=nullptr);
    ~MetadataEditWidget_Commentators();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private:
    void addCommentatorUI(const QString& name, const QString& social, const QString& pronouns);

private slots:
    void onAddCommentatorReleased();

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
