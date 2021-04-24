#pragma once

#include "uh/AnalyzerPlugin.hpp"
#include <QWidget>

class HitAnalyzerData;

class HitAnalyzer : public QWidget
                  , public uh::AnalyzerPlugin
{
    Q_OBJECT

public:
    explicit HitAnalyzer(QWidget* parent=nullptr);
    ~HitAnalyzer();

    void processDataSet(const uh::DataSet* dataSet) override;

private:
    HitAnalyzerData* d;
};
