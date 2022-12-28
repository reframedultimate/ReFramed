#include "data-viewer/models/VideoMetadataModel.hpp"

#include "rfcommon/VideoMeta.hpp"

// ----------------------------------------------------------------------------
void VideoMetadataModel::setVideoMetadata(rfcommon::VideoMeta* vmeta)
{
    beginResetModel();
    endResetModel();
}

// ----------------------------------------------------------------------------
int VideoMetadataModel::rowCount(const QModelIndex& parent) const
{
    return 0;
}

// ----------------------------------------------------------------------------
int VideoMetadataModel::columnCount(const QModelIndex& parent) const
{
    return 0;
}

// ----------------------------------------------------------------------------
QVariant VideoMetadataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant VideoMetadataModel::data(const QModelIndex& index, int role) const
{
    return QVariant();
}
