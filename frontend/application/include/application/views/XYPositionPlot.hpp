#pragma once

#include "application/views/RealtimePlot.hpp"
#include "uh/RecordingListener.hpp"
#include "uh/Reference.hpp"
#include <QExplicitlySharedDataPointer>

class QActionGroup;
class QwtPlotDirectPainter;
class QwtPlotCurve;

namespace uh {
    class Recording;
}

namespace uhapp {

class XYPositionPlotContextMenuActions;

class XYPositionPlot : public RealtimePlot
                     , public uh::RecordingListener
{
    Q_OBJECT
public:
    explicit XYPositionPlot(QWidget* parent=nullptr);
    ~XYPositionPlot();

public slots:
    void clear();
    void setRecording(uh::Recording* recording);

protected:
    void prependContextMenuActions(QMenu* menu) override;

private:
    void onActiveRecordingPlayerNameChanged(int player, const std::string& name) override;
    void onActiveRecordingNewUniquePlayerState(int player, const uh::PlayerState& state) override;

    void onActiveRecordingSetNumberChanged(uh::SetNumber number) override { (void)number; }
    void onActiveRecordingGameNumberChanged(uh::GameNumber number) override { (void)number; }
    void onActiveRecordingFormatChanged(const uh::SetFormat& format) { (void)format; }
    void onActiveRecordingNewPlayerState(int player, const uh::PlayerState& state) override { (void)player; (void)state; }
    void onRecordingWinnerChanged(int winner) override { (void)winner; }

private slots:
    void onDottedAction(bool enable);
    void onLinesAction(bool enable);
    void setCurveVisible(int player, bool visible);

private:
    QVector<QwtPlotCurve*> curves_;
    uh::Reference<uh::Recording> recording_;
    QActionGroup* curveTypeActionGroup_;
};

}
