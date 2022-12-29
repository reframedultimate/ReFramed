#include "data-viewer/models/VideoMetadataModel.hpp"

#include "rfcommon/VideoMeta.hpp"

#include <QTime>

// ----------------------------------------------------------------------------
VideoMetadataModel::~VideoMetadataModel()
{
    if (vmeta_)
        vmeta_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void VideoMetadataModel::setVideoMetadata(rfcommon::VideoMeta* vmeta)
{
    beginResetModel();
        if (vmeta_)
            vmeta_->dispatcher.removeListener(this);

        vmeta_ = vmeta;

        if (vmeta_)
            vmeta_->dispatcher.addListener(this);
    endResetModel();
}

// ----------------------------------------------------------------------------
int VideoMetadataModel::rowCount(const QModelIndex& parent) const
{
    if (vmeta_)
        return 4;
    return 0;
}

// ----------------------------------------------------------------------------
int VideoMetadataModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

// ----------------------------------------------------------------------------
QVariant VideoMetadataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch (section)
        {
            case 0: return "Key";
            case 1: return "Value";
        }
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant VideoMetadataModel::data(const QModelIndex& index, int role) const
{
    switch (role)
    {
        case Qt::DisplayRole:
            if (index.column() == 0)
                switch (index.row())
                {
                    case 0: return "File Name";
                    case 1: return "Frame Offset";
                    case 2: return "Time Offset";
                    case 3: return "Embedded";
                }
            if (index.column() == 1)
                switch (index.row())
                {
                    case 0: return QString::fromUtf8(vmeta_->fileName());
                    case 1: return QString::number(vmeta_->frameOffset().index());
                    case 2: return QTime(0, 0).addMSecs(vmeta_->frameOffset().millisPassed()).toString();
                    case 3: return vmeta_->isEmbedded() ? "True" : "False";
                }
            break;

        case Qt::TextAlignmentRole:
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
