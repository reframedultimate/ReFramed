#include "data-viewer/models/FighterStatesModel.hpp"

#include "rfcommon/FighterState.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/MotionLabels.hpp"

#include <algorithm>

enum ColumnType
{
    FrameIndex,
    TimePassed,
    FramesLeft,
    TimeLeft,
    Position,
    Facing,
    Damage,
    Hitstun,
    Shield,
    Status,
    StatusName,
    Hash40,
    Hash40String,
    MotionLabel,
    HitStatus,
    HitStatusName,
    Stocks,
    AttackConnected,

    ColumnCount
};

// ----------------------------------------------------------------------------
FighterStatesModel::FighterStatesModel(
        rfcommon::FrameData* frameData,
        rfcommon::MappingInfo* mappingInfo,
        int fighterIdx,
        rfcommon::FighterID fighterID,
        rfcommon::MotionLabels* labels)
    : frameData_(frameData)
    , mappingInfo_(mappingInfo)
    , labels_(labels)
    , fighterIdx_(fighterIdx)
    , fighterID_(fighterID)
{
    labels_->dispatcher.addListener(this);
    frameData_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
FighterStatesModel::~FighterStatesModel()
{
    frameData_->dispatcher.removeListener(this);
    labels_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
int FighterStatesModel::rowCount(const QModelIndex& parent) const
{
    PROFILE(FighterStatesModel, rowCount);

    return frameData_->frameCount();
}

// ----------------------------------------------------------------------------
int FighterStatesModel::columnCount(const QModelIndex& parent) const
{
    PROFILE(FighterStatesModel, columnCount);

    return ColumnCount;
}

// ----------------------------------------------------------------------------
QVariant FighterStatesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    PROFILE(FighterStatesModel, headerData);

    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
                case FrameIndex: return "Frame";
                case TimePassed: return "Time";
                case FramesLeft: return "Left";
                case TimeLeft: return "Timer";
                case Position: return "Position";
                case Facing: return "Facing";
                case Damage: return "Dmg (%)";
                case Hitstun: return "Hitstun";
                case Shield: return "Shield";
                case Status: return "Status";
                case StatusName: return "Status Name";
                case Hash40: return "Hash40";
                case Hash40String: return "String";
                case MotionLabel: return "Motion Label";
                case HitStatus: return "Hit Status";
                case HitStatusName: return "Hit Status Name";
                case Stocks: return "Stocks";
                case AttackConnected: return "Attack Connected";
            }
        }
        else if (orientation == Qt::Vertical)
        {
            const rfcommon::FighterState& state = frameData_->stateAt(fighterIdx_, section);
            return QString::number(state.frameIndex().index());
        }
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant FighterStatesModel::data(const QModelIndex& index, int role) const
{
    PROFILE(FighterStatesModel, data);

    switch (role)
    {
        case Qt::DisplayRole: {
            const rfcommon::FighterState& state = frameData_->stateAt(fighterIdx_, index.row());

            auto formatTimer = [](const double timeS) -> QString {
                const int m = static_cast<int>(timeS/60);
                const int s = static_cast<int>(timeS) - m*60;
                const int hundredths = static_cast<int>(timeS*100) - s*100 - m*60*100;
                return QString::number(m) + ":" + QString::number(s) + "." + QString::number(hundredths).rightJustified(2, '0');
            };

            auto formatPosition = [](const rfcommon::Vec2& pos) -> QString {
                return "[" + QString::number(pos.x(), 'f', 2) + ", " + QString::number(pos.y(), 'f', 2) + "]";
            };

            auto statusName = [this](rfcommon::FighterStatus status) -> QString {
                if (status.isValid() == false)
                    return "(Invalid Status ID)";
                if (fighterID_.isValid() == false)
                    return "(Fighter ID missing)";
                if (mappingInfo_.isNull())
                    return "(Mapping info missing)";
                return mappingInfo_->status.toName(fighterID_, status);
            };

            auto formatHitStatusName = [this](rfcommon::FighterHitStatus hitStatus) -> QString {
                if (hitStatus.isValid() == false)
                    return "(Invalid Hit Status ID)";
                if (mappingInfo_.isNull())
                    return "(Mapping info missing)";
                return mappingInfo_->hitStatus.toName(hitStatus);
            };

            auto formatMotionLabels = [this](rfcommon::FighterID fighterID, rfcommon::FighterMotion motion) -> QString {
                QString list;
                for (int i = 0; i != labels_->layerCount(); ++i)
                {
                    if (fighterID.isValid() == false)
                    {
                        if (i != 0)
                            list += ", ";
                        if (const char* h40 = labels_->toHash40(motion))
                            list += h40;
                        else
                            list += motion.toHex().cStr();
                        continue;
                    }

                    if (const char* label = labels_->toLabel(fighterID, motion, i))
                    {
                        if (i != 0)
                            list += ", ";
                        list += label;
                    }
                }

                return list;
            };

            switch (index.column())
            {
                case FrameIndex: return QString::number(state.frameIndex().index());
                case TimePassed: return formatTimer(state.frameIndex().secondsPassed());
                case FramesLeft: return QString::number(state.framesLeft().count());
                case TimeLeft: return formatTimer(state.framesLeft().secondsLeft());
                case Position: return formatPosition(state.pos());
                case Facing: return state.flags().facingLeft() ? "Right" : "Left";
                case Damage: return QString::number(state.damage(), 'f', 1);
                case Hitstun: return QString::number(state.hitstun(), 'f', 1);
                case Shield: return QString::number(state.shield(), 'f', 1);
                case Status: return QString::number(state.status().value());
                case StatusName: return statusName(state.status());
                case Hash40: return "0x" + QString::number(state.motion().value(), 16);
                case Hash40String: return labels_->toHash40(state.motion(), "(unknown)");
                case MotionLabel: return formatMotionLabels(fighterID_, state.motion());
                case HitStatus: return QString::number(state.hitStatus().value());
                case HitStatusName: return formatHitStatusName(state.hitStatus());
                case Stocks: return QString::number(state.stocks().count());
                case AttackConnected: return state.flags().attackConnected() ? "True" : "False";
            }
        } break;

        case Qt::TextAlignmentRole: {
            switch (index.column())
            {
                case StatusName:
                case Hash40String:
                case MotionLabel:
                case HitStatusName:
                    return static_cast<Qt::Alignment::Int>(Qt::AlignLeft | Qt::AlignVCenter);

                default:
                    return static_cast<Qt::Alignment::Int>(Qt::AlignHCenter | Qt::AlignVCenter);
            }
        } break;

        case Qt::CheckStateRole:
        case Qt::DecorationRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
        case Qt::SizeHintRole:
            break;
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
void FighterStatesModel::onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame)
{
    PROFILE(FighterStatesModel, onFrameDataNewUniqueFrame);

}

// ----------------------------------------------------------------------------
void FighterStatesModel::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame)
{
    PROFILE(FighterStatesModel, onFrameDataNewFrame);

    beginInsertRows(QModelIndex(), frameIdx, frameIdx);
    endInsertRows();
}

// ----------------------------------------------------------------------------
void FighterStatesModel::updateMotionLabelsColumn()
{
    PROFILE(FighterStatesModel, updateMotionUserLabelsColumn);

    emit dataChanged(index(0, MotionLabel), index(frameData_->frameCount() - 1, MotionLabel));
}
void FighterStatesModel::updateHash40StringsColumn()
{
    PROFILE(FighterStatesModel, updateHash40StringsColumn);

    emit dataChanged(index(0, Hash40String), index(frameData_->frameCount() - 1, Hash40String));
}
void FighterStatesModel::onMotionLabelsLoaded() { updateMotionLabelsColumn(); }
void FighterStatesModel::onMotionLabelsHash40sUpdated() { updateHash40StringsColumn(); }

void FighterStatesModel::onMotionLabelsPreferredLayerChanged(int usage) {}

void FighterStatesModel::onMotionLabelsLayerInserted(int layerIdx) { updateMotionLabelsColumn(); }
void FighterStatesModel::onMotionLabelsLayerRemoved(int layerIdx) { updateMotionLabelsColumn(); }
void FighterStatesModel::onMotionLabelsLayerNameChanged(int layerIdx) {}
void FighterStatesModel::onMotionLabelsLayerUsageChanged(int layerIdx, int oldUsage) {}
void FighterStatesModel::onMotionLabelsLayerMoved(int fromIdx, int toIdx) { updateMotionLabelsColumn(); }
void FighterStatesModel::onMotionLabelsLayerMerged(int layerIdx) { updateMotionLabelsColumn(); }

void FighterStatesModel::onMotionLabelsRowInserted(rfcommon::FighterID fighterID, int row) { updateMotionLabelsColumn(); }
void FighterStatesModel::onMotionLabelsLabelChanged(rfcommon::FighterID fighterID, int row, int layerIdx) { updateMotionLabelsColumn(); }
void FighterStatesModel::onMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int row, int oldCategory) {}
