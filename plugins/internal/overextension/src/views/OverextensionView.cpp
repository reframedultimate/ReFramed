#include "overextension/views/OverextensionView.hpp"
#include "overextension/models/OverextensionModel.hpp"

// ----------------------------------------------------------------------------
OverextensionView::OverextensionView(OverextensionModel* model)
    : model_(model)
{
    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
OverextensionView::~OverextensionView()
{
    model_->dispatcher.removeListener(this);
}
