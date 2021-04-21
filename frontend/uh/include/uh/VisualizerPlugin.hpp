#pragma once

class QWidget;

namespace uh {

class Plugin
{
public:
    virtual QWidget* getWidget() = 0;
};

class VisualizerPlugin : public Plugin
{
public:
};

class AnalyzerPlugin : public Plugin
{
public:
};

class TrainingModePlugin : public Plugin
{
public:
};

}
