#pragma once

#include "uh/config.hpp"
#include "uh/RefCounted.hpp"

namespace uh {

class Recording;
class DataPoint;
class PlayerState;

class UH_PUBLIC_API DataSet : public RefCounted
{
public:
    DataSet();
    ~DataSet();
    DataSet(const DataSet& other);
    DataSet(DataSet&& other);
    DataSet& operator=(DataSet rhs);
    DataSet& operator=(DataSet&& rhs);
    friend void swap(DataSet& first, DataSet& second) noexcept;

    void appendPlayerStatesFromRecording(int player, Recording* recording);
    void appendPlayerState(Recording* recording, const PlayerState& state);
    void erase(const DataPoint* dp);
    void erase(int idx);
    void clear();

    int count() const;
    const DataPoint* begin() const;
    const DataPoint* end() const;
    DataPoint* begin();
    DataPoint* end();

private:
    char* mem_;
    int count_;
    int capacity_;
};

}
