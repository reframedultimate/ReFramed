#include "ui_HitAnalyzer.h"
#include "hit-analysis/HitAnalyzer.hpp"
#include "rfcommon/DataSet.hpp"
#include "rfcommon/DataPoint.hpp"
#include "rfcommon/SavedGameSession.hpp"

// ----------------------------------------------------------------------------
HitAnalyzer::HitAnalyzer(RFPluginFactory* factory, QWidget *parent)
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
void HitAnalyzer::setPointOfView(const rfcommon::String& playerName)
{

}

// ----------------------------------------------------------------------------
void HitAnalyzer::dataSetPreparing(float progress, const rfcommon::String& info)
{

}

// ----------------------------------------------------------------------------
void HitAnalyzer::dataSetCancelled()
{

}

// ----------------------------------------------------------------------------
void HitAnalyzer::processDataSet(const rfcommon::DataSet* dataSet)
{

}
