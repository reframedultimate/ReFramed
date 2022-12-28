#pragma once

#include <QAbstractTableModel>
#include "rfcommon/Reference.hpp"
#include "rfcommon/VideoMetadataListener.hpp"

namespace rfcommon {
    class VideoMeta;
}

class VideoMetadataModel
        : public QAbstractTableModel
        , rfcommon::VideoMetadataListener
{
public:
    void setVideoMetadata(rfcommon::VideoMeta* vmeta);

    int rowCount(const QModelIndex& parent=QModelIndex()) const override;
    int columnCount(const QModelIndex& parent=QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const override;

private:
    rfcommon::Reference<rfcommon::VideoMeta> vmeta_;
};
