#pragma once

#include <QWidget>

class QVBoxLayout;

namespace Ui {
    class DataSetFilterView;
}

namespace uhapp {

class DataSetFilterWidget;

class DataSetFilterView : public QWidget
{
    Q_OBJECT

public:
    explicit DataSetFilterView(QWidget* parent=nullptr);
    ~DataSetFilterView();

protected:
    virtual bool eventFilter(QObject* target, QEvent* e) override;

private slots:
    void onToolButtonAddFilterTriggered(QAction* action);

private:
    void addNewFilterWidget(DataSetFilterWidget* widget);
    void recursivelyInstallEventFilter(QObject* obj);

private:
    Ui::DataSetFilterView* ui_;
    QVBoxLayout* filterWidgetsLayout_;
};

}
