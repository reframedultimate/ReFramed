#include "stage-stats/views/StageStatsView.hpp"
#include "stage-stats/models/StageStatsModel.hpp"

#include "rfcommon/Profiler.hpp"

#include <QGridLayout>
#include <QLayoutItem>
#include <QLabel>
#include <QSpacerItem>

// ----------------------------------------------------------------------------
static void clearLayout(QLayout* layout)
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
StageStatsView::StageStatsView(StageStatsModel* model, QWidget* parent)
    : QWidget(parent)
    , model_(model)
    , layout_(new QGridLayout)
{
    setLayout(layout_);

    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
StageStatsView::~StageStatsView()
{
    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void StageStatsView::onDataUpdated()
{
    PROFILE(StageStatsView, onDataUpdated);

    clearLayout(layout_);

    layout_->addWidget(new QLabel("Top 3 Stages"), 0, 1);
    for (int fighterIdx = 0; fighterIdx != model_->fighterCount(); ++fighterIdx)
    {
        QString top3;
        for (const auto& stageID : model_->top3Stages(fighterIdx))
        {
            if (top3.length() > 0)
                top3 += ", ";
            top3 += model_->stageName(stageID);
        }

        QString name = model_->playerName(fighterIdx);
        name += " (";
        name += model_->characterName(fighterIdx);
        name += ")";

        layout_->addWidget(new QLabel(name), fighterIdx + 1, 0);
        layout_->addWidget(new QLabel(top3), fighterIdx + 1, 1);
    }

    layout_->addItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding), model_->fighterCount() + 1, 0);
    layout_->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 2);
}
