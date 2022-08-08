#pragma once

class XYPositionsPlotListener
{
public:
    virtual void onDataSetChanged() = 0;
    virtual void onDataChanged() = 0;
    virtual void onNamesChanged() = 0;
};
