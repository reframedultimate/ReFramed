#include "data-viewer/models/FighterStatesModel.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/FighterState.hpp"
#include "rfcommon/FrameData.hpp"
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
    Motion,
    MotionLabel,
    MotionUserLabel,
    HitStatus,
    HitStatusName,
    Stocks,
    AttackConnected,

    ColumnCount
};

// ----------------------------------------------------------------------------
FighterStatesModel::FighterStatesModel(rfcommon::FrameData* frameData, rfcommon::MappingInfo* mappingInfo, int fighterIdx, rfcommon::FighterID fighterID)
    : frameData_(frameData)
    , mappingInfo_(mappingInfo)
    , fighterIdx_(fighterIdx)
    , fighterID_(fighterID)
{
    frameData_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
FighterStatesModel::~FighterStatesModel()
{
    frameData_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
int FighterStatesModel::rowCount(const QModelIndex& parent) const
{
    return frameData_->frameCount();
}

// ----------------------------------------------------------------------------
int FighterStatesModel::columnCount(const QModelIndex& parent) const
{
    return ColumnCount;
}

// ----------------------------------------------------------------------------
QVariant FighterStatesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
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
                case Motion: return "Motion";
                case MotionLabel: return "Motion Label";
                case MotionUserLabel: return "User Label";
                case HitStatus: return "Hit Status";
                case HitStatusName: return "Hit Status Name";
                case Stocks: return "Stocks";
                case AttackConnected: return "Attack Connected";
            }
        }
        else if (orientation == Qt::Vertical)
        {
            const rfcommon::FighterState& state = frameData_->stateAt(fighterIdx_, section);
            return QString::number(state.frameIndex().value());
        }
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant FighterStatesModel::data(const QModelIndex& index, int role) const
{
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

            auto formatPosition = [](double x, double y) -> QString {
                return "[" + QString::number(x, 'f', 2) + ", " + QString::number(y, 'f', 2) + "]";
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

            switch (index.column())
            {
                case FrameIndex: return QString::number(state.frameIndex().value());
                case TimePassed: return formatTimer(state.frameIndex().secondsPassed());
                case FramesLeft: return QString::number(state.framesLeft().value());
                case TimeLeft: return formatTimer(state.framesLeft().secondsLeft());
                case Position: return formatPosition(state.posx(), state.posy());
                case Facing: return state.flags().facingLeft() ? "Left" : "Right";
                case Damage: return QString::number(state.damage(), 'f', 1);
                case Hitstun: return QString::number(state.hitstun(), 'f', 1);
                case Shield: return QString::number(state.shield(), 'f', 1);
                case Status: return QString::number(state.status().value());
                case StatusName: return statusName(state.status());
                case Motion: return "0x" + QString::number(state.motion().value(), 16);
                case MotionLabel: return "";
                case MotionUserLabel: return "";
                case HitStatus: return "";
                case HitStatusName: return formatHitStatusName(state.hitStatus());
                case Stocks: return QString::number(state.stocks().value());
                case AttackConnected: return state.flags().attackConnected() ? "True" : "False";
            }
        } break;

        case Qt::TextAlignmentRole:
            switch (index.column())
            {
                case StatusName:
                case MotionLabel:
                case MotionUserLabel:
                case HitStatusName:
                    return Qt::AlignLeft + Qt::AlignVCenter;

                default:
                    return Qt::AlignHCenter + Qt::AlignVCenter;
            }

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
}

// ----------------------------------------------------------------------------
void FighterStatesModel::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame)
{
    beginInsertRows(QModelIndex(), frameIdx, frameIdx);
    endInsertRows();
}
