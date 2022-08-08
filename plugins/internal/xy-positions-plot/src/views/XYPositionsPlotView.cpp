#include "xy-positions-plot/models/XYPositionsPlotCurveData.hpp"
#include "xy-positions-plot/models/XYPositionsPlotModel.hpp"
#include "xy-positions-plot/views/XYPositionsPlotView.hpp"
#include "xy-positions-plot/widgets/XYPositionsPlot.hpp"

#include "rfcommon/Frame.hpp"

#include "qwt_plot_curve.h"

#include <QListView>
#include <QSplitter>
#include <QVboxLayout>
#include <QMenu>
#include <QAction>
#include <QPen>

// ----------------------------------------------------------------------------
XYPositionsPlotView::XYPositionsPlotView(XYPositionsPlotModel* model, QWidget* parent)
    : QWidget(parent)
    , model_(model)
    , splitter_(new QSplitter)
    , plot_(new XYPositionsPlot(model))
    , sessionsList_(new QListView)
    , lastSessionCount_(0)
{
    plot_->setTitle("XY Positions");
    plot_->buildCurves();

    splitter_->addWidget(plot_);

    setLayout(new QVBoxLayout);
    layout()->addWidget(splitter_);

    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
XYPositionsPlotView::~XYPositionsPlotView()
{
    model_->dispatcher.removeListener(this);
    delete sessionsList_;
}

// ----------------------------------------------------------------------------
void XYPositionsPlotView::onDataSetChanged() 
{
    if (model_->sessionCount() == 1)
    {
        plot_->setParent(nullptr);
        sessionsList_->setParent(nullptr);
        splitter_->addWidget(plot_);
    }
    else if (lastSessionCount_ == 1 && model_->sessionCount() > 1)
    {
        plot_->setParent(nullptr);
        sessionsList_->setParent(nullptr);

        splitter_->addWidget(sessionsList_);
        splitter_->addWidget(plot_);
        splitter_->setStretchFactor(0, 0);
        splitter_->setStretchFactor(1, 1);
        splitter_->setSizes({440, 1});
    }
    lastSessionCount_ = model_->sessionCount();
}

// ----------------------------------------------------------------------------
void XYPositionsPlotView::onDataChanged() {}
void XYPositionsPlotView::onNamesChanged() {}
