#pragma once

#include "uh/AnalyzerPlugin.hpp"
#include <QWidget>

namespace Ui {
    class HitAnalyzer;
}

class HitAnalyzer : public QWidget
                  , public uh::AnalyzerPlugin
{
    Q_OBJECT

public:
    explicit HitAnalyzer(QWidget* parent=nullptr);
    ~HitAnalyzer();

    void setPointOfView(const uh::String& playerName) override;
    void dataSetPreparing(float progress, const uh::String& info) override;
    void dataSetCancelled() override;
    void processDataSet(const uh::DataSet* dataSet) override;

private:
    Ui::HitAnalyzer* ui_;
};
