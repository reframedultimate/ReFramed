#pragma once

class QWidget;

namespace uh {

class Plugin
{
public:
    virtual ~Plugin() {}
    virtual QWidget* takeWidget() = 0;
    virtual void giveWidget() = 0;
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
