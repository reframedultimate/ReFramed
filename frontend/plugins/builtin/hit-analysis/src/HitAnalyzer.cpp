#include "hit-analysis/ui_HitAnalyzer.h"
#include "hit-analysis/HitAnalyzer.hpp"
#include "uh/DataSet.hpp"
#include "uh/DataPoint.hpp"
#include "uh/SavedGameSession.hpp"

// ----------------------------------------------------------------------------
HitAnalyzer::HitAnalyzer(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::HitAnalyzer)
{
    ui_->setupUi(this);
}

// ----------------------------------------------------------------------------
HitAnalyzer::~HitAnalyzer()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void HitAnalyzer::setPointOfView(const uh::String& playerName)
{

}

// ----------------------------------------------------------------------------
void HitAnalyzer::dataSetPreparing(float progress, const uh::String& info)
{

}

// ----------------------------------------------------------------------------
void HitAnalyzer::dataSetCancelled()
{

}

// ----------------------------------------------------------------------------
void HitAnalyzer::processDataSet(const uh::DataSet* dataSet)
{

}

// ----------------------------------------------------------------------------
QWidget* HitAnalyzer::createView()
{
    return this;
}

// ----------------------------------------------------------------------------
void HitAnalyzer::destroyView(QWidget* widget)
{

}

