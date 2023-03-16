#include "application/ui_PropagateLabelsDialog.h"
#include "application/views/PropagateLabelsDialog.hpp"

namespace rfapp {

// ----------------------------------------------------------------------------
PropagateLabelsDialog::PropagateLabelsDialog(QWidget* parent, bool overwriteExisting, bool forceCreation)
	: QDialog(parent)
	, ui_(new Ui::PropagateLabelsDialog)
{
	ui_->setupUi(this);

	ui_->checkBox_overwrite->setChecked(overwriteExisting);
	ui_->checkBox_createNew->setChecked(forceCreation);

	connect(ui_->pushButton_propagate, &QPushButton::released, [this] { done(QDialog::Accepted); });
	connect(ui_->pushButton_cancel, &QPushButton::released, [this] { done(QDialog::Rejected); });
}

// ----------------------------------------------------------------------------
PropagateLabelsDialog::~PropagateLabelsDialog()
{
	delete ui_;
}

// ----------------------------------------------------------------------------
bool PropagateLabelsDialog::overwriteExisting() const
{
	return ui_->checkBox_overwrite->isChecked();
}

// ----------------------------------------------------------------------------
bool PropagateLabelsDialog::forceCreation() const
{
	return ui_->checkBox_createNew->isChecked();
}

}
