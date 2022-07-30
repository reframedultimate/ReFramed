#include "rfcommon/Frame.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/FighterState.hpp"
#include "rfcommon/StreamBuffer.hpp"
#include <cassert>
#include "zlib.h"

namespace rfcommon {

static FrameData* load_1_5(StreamBuffer* data);

// ----------------------------------------------------------------------------
FrameData::FrameData(SmallVector<Vector<FighterState>, 2>&& fighterFrames)
    : fighters_(std::move(fighterFrames))
{}

// ----------------------------------------------------------------------------
FrameData::FrameData(int fighterCount)
    : fighters_(fighterCount)
{}

// ----------------------------------------------------------------------------
FrameData::~FrameData()
{}

// ----------------------------------------------------------------------------
FrameData* FrameData::load(FILE* fp, uint32_t size)
{
    StreamBuffer compressed(size);
    size_t bytesRead = fread(compressed.writeToPtr(size), 1, size, fp);
    if (bytesRead != (size_t)size)
        return nullptr;

    int error = 0;
    const uint8_t major = compressed.readU8(&error); if (error) return nullptr;
    const uint8_t minor = compressed.readU8(&error); if (error) return nullptr;

    if (major == 1 && minor == 5)
    {
        uLongf decompressedSize = compressed.readLU32(&error);
        if (error)
            return nullptr;

        StreamBuffer buffer(decompressedSize);
        int result = uncompress(
                static_cast<uint8_t*>(buffer.writeToPtr(decompressedSize)), &decompressedSize,
                static_cast<const uint8_t*>(compressed.get()) + 6, compressed.capacity() - 6);
        if (result != Z_OK)
            return nullptr;
        if (decompressedSize != (uLongf)buffer.capacity())
            return nullptr;
        return load_1_5(&buffer);
    }

    // Unsupported version
    return nullptr;
}
FrameData* load_1_5(StreamBuffer* data)
{
    int error = 0;
    FramesLeft::Type frameCount = data->readLU32(&error);
    int fighterCount = data->readU8(&error);
    if (error)
        return nullptr;

    SmallVector<Vector<FighterState>, 2> frames(fighterCount);
    for (int fighter = 0; fighter != fighterCount; ++fighter)
        for (FramesLeft::Type frame = 0; frame < frameCount; ++frame)
        {
            const auto timeStamp = TimeStamp::fromMillisSinceEpoch(data->readLU64(&error));
            const auto framesLeft = FramesLeft::fromValue(data->readLU32(&error));
            const float posx = data->readLF32(&error);
            const float posy = data->readLF32(&error);
            const float damage = data->readLF32(&error);
            const float hitstun = data->readLF32(&error);
            const float shield = data->readLF32(&error);
            const auto status = FighterStatus::fromValue(data->readLU16(&error));
            const uint32_t motion_l = data->readLU32(&error);
            const uint8_t motion_h = data->readU8(&error);
            const auto motion = FighterMotion::fromParts(motion_h, motion_l);
            const auto hitStatus = FighterHitStatus::fromValue(data->readU8(&error));
            const auto stocks = FighterStocks::fromValue(data->readU8(&error));
            const auto flags = FighterFlags::fromValue(data->readU8(&error));

            if (error)
                return nullptr;

            frames[fighter].emplace(
                timeStamp,
                FrameNumber::fromValue(frame),
                framesLeft,
                posx,
                posy,
                damage,
                hitstun,
                shield,
                status,
                motion,
                hitStatus,
                stocks,
                flags);
        }

    return new FrameData(std::move(frames));
}

// ----------------------------------------------------------------------------
uint32_t FrameData::save(FILE* fp) const
{
    const int fighterStateSize =
            sizeof(TimeStamp::Type) +
            sizeof(FramesLeft::Type) +
            sizeof(fighters_[0][0].posx()) +
            sizeof(fighters_[0][0].posy()) +
            sizeof(fighters_[0][0].damage()) +
            sizeof(fighters_[0][0].hitstun()) +
            sizeof(fighters_[0][0].shield()) +
            sizeof(FighterStatus::Type) +
            5 + // motion is a hash40 (40 bits)
            sizeof(FighterHitStatus::Type) +
            sizeof(FighterStocks::Type) +
            sizeof(FighterFlags::Type);
    const int frameSize = fighterStateSize * fighterCount();

    StreamBuffer buffer(5 + frameSize * frameCount());
    buffer.writeLU32(frameCount());
    buffer.writeU8(fighterCount());
    for (const auto& fighter : fighters_)
        for (const auto& frame : fighter)
        {
            buffer
                .writeLU64(frame.timeStamp().millisSinceEpoch())
                .writeLU32(frame.framesLeft().value())
                .writeLF32(frame.posx())
                .writeLF32(frame.posy())
                .writeLF32(frame.damage())
                .writeLF32(frame.hitstun())
                .writeLF32(frame.shield())
                .writeLU16(frame.status().value())
                .writeLU32(frame.motion().lower())
                .writeU8(frame.motion().upper())
                .writeU8(frame.hitStatus().value())
                .writeU8(frame.stocks().value())
                .writeU8(frame.flags().value());
        }
    assert(buffer.bytesWritten() == 5 + frameSize * frameCount());

    uLongf compressedSize = compressBound(buffer.bytesWritten());
    StreamBuffer compressed(compressedSize + 6);
    compressed.writeU8(1);  // Major version
    compressed.writeU8(5);  // Minor version
    compressed.writeLU32(buffer.bytesWritten());  // Decompressed size
    if (compress2(
            static_cast<uint8_t*>(compressed.writeToPtr(compressedSize)), &compressedSize,
            static_cast<const uint8_t*>(buffer.get()), buffer.bytesWritten(), 9) != Z_OK)
    {
        return 0;
    }

    if (fwrite(compressed.get(), 1, compressed.bytesWritten(), fp) != compressed.bytesWritten())
        return 0;

    return compressed.bytesWritten();
}

// ----------------------------------------------------------------------------
int FrameData::fighterCount() const
{
    return fighters_.count();
}

// ----------------------------------------------------------------------------
int FrameData::frameCount() const
{
    return fighters_.count() ? fighters_[0].count() : 0;
}

// ----------------------------------------------------------------------------
const FighterState& FrameData::stateAt(int frameIdx, int fighterIdx) const
{
    assert(fighterIdx < fighterCount());
    assert(frameIdx < frameCount());
    return fighters_[fighterIdx][frameIdx];
}

// ----------------------------------------------------------------------------
const FighterState& FrameData::firstState(int fighterIdx) const
{
    assert(frameCount() > 0);
    return fighters_[fighterIdx].front();
}

// ----------------------------------------------------------------------------
const FighterState& FrameData::lastState(int fighterIdx) const
{
    assert(frameCount() > 0);
    return fighters_[fighterIdx].back();
}

// ----------------------------------------------------------------------------
void FrameData::addFrame(Frame&& frame)
{
    // Sanity checks
    assert(frame.count() == fighterCount());
#ifndef NDEBUG
    for (int i = 1; i < fighterCount(); ++i)
    {
        assert(frame[0].framesLeft() == frame[i].framesLeft());
        assert(frame[0].frameNumber() == frame[i].frameNumber());
    }
#endif

    for (int i = 0; i < fighterCount(); ++i)
        fighters_[i].push(frame[i]);

    // If any fighter state is different from the previous one, notify
    if (frameCount() < 2 || framesHaveSameData(frameCount() - 1, frameCount() - 2))
        dispatcher.dispatch(&FrameDataListener::onFrameDataNewUniqueFrame, frameCount() - 1, frame);

    // The UI cares about every frame
    dispatcher.dispatch(&FrameDataListener::onFrameDataNewFrame, frameCount() - 1, frame);
}

// ----------------------------------------------------------------------------
bool FrameData::framesHaveSameData(int frameIdx1, int frameIdx2) const
{
    // Compare each fighter state between each frame. If they are all equal,
    // then it means both frames compare equal
    for (int i = 0; i != fighterCount(); ++i)
        if (fighters_[i][frameIdx1].hasSameDataAs(fighters_[i][frameIdx2]) == false)
            return false;
    return true;
}

}
