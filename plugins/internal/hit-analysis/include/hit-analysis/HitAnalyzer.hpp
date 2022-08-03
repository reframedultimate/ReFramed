#pragma once

#include "rfcommon/AnalyzerPlugin.hpp"
#include <QWidget>

namespace Ui {
    class HitAnalyzer;
}

class HitAnalyzer : public QWidget
                  , public rfcommon::AnalyzerPlugin
{
    Q_OBJECT

public:
    explicit HitAnalyzer(RFPluginFactory* factory, QWidget* parent=nullptr);
    ~HitAnalyzer();

    QWidget* createView() override;
    void destroyView(QWidget* view) override;

    void setPointOfView(const rfcommon::String& playerName) override;
    void dataSetPreparing(float progress, const rfcommon::String& info) override;
    void dataSetCancelled() override;
    void processDataSet(const rfcommon::DataSet* dataSet) override;

private:
    Ui::HitAnalyzer* ui_;
};
