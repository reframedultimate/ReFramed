#include "hit-analysis/HitAnalyzer.hpp"
#include "uh/DataSet.hpp"
#include "uh/DataPoint.hpp"

class HitAnalyzerData
{
public:
};

// ----------------------------------------------------------------------------
HitAnalyzer::HitAnalyzer(QWidget *parent)
    : QWidget(parent)
    , d(new HitAnalyzerData)
{
}

// ----------------------------------------------------------------------------
HitAnalyzer::~HitAnalyzer()
{
    delete d;
}

// ----------------------------------------------------------------------------
void HitAnalyzer::processDataSet(const uh::DataSet* dataSet)
{

}
