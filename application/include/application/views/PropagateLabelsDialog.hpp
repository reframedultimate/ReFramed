#pragma once

#include <QDialog>

namespace Ui {
	class PropagateLabelsDialog;
}

namespace rfapp {

class PropagateLabelsDialog : public QDialog
{
	Q_OBJECT

public:
	PropagateLabelsDialog(QWidget* parent, bool overwriteExisting=false, bool forceCreation=false);
	~PropagateLabelsDialog();

	bool overwriteExisting() const;
	bool forceCreation() const;

private:
	Ui::PropagateLabelsDialog* ui_;
};

}
