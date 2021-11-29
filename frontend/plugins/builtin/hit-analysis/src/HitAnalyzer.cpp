#include "hit-analysis/ui_HitAnalyzer.h"
#include "hit-analysis/HitAnalyzer.hpp"
#include "uh/DataSet.hpp"
#include "uh/DataPoint.hpp"
#include "uh/SavedGameSession.hpp"

// ----------------------------------------------------------------------------
HitAnalyzer::HitAnalyzer(UHPluginFactory* factory, QWidget *parent)
    : AnalyzerPlugin(factory)
    , QWidget(parent)
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
QWidget* HitAnalyzer::createView()
{
    return this;
}

// ----------------------------------------------------------------------------
void HitAnalyzer::destroyView(QWidget* view)
{
    (void)view;
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
