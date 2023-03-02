#include "rfcommon/Deserializer.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/FighterState.hpp"
#include "rfcommon/MemoryBuffer.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Serializer.hpp"
#include <cassert>
#include "zlib.h"

namespace rfcommon {

static FrameData* load_1_5(Deserializer& data);

// ----------------------------------------------------------------------------
FrameData::FrameData(SmallVector<Vector<FighterState>, 2>&& fighterFrames)
    : fighters_(std::move(fighterFrames))
{}

// ----------------------------------------------------------------------------
FrameData::FrameData(int fighterCount)
    : fighters_(SmallVector<Vector<FighterState>, 2>::makeResized(fighterCount))
{}

// ----------------------------------------------------------------------------
FrameData::~FrameData()
{}

// ----------------------------------------------------------------------------
FrameData* FrameData::load(const void* data, uint64_t len)
{
    PROFILE(FrameData, load);

    Deserializer compressed(data, len);
    const uint8_t major = compressed.readU8();
    const uint8_t minor = compressed.readU8();

    if (major == 1 && minor == 5)
    {
        uLongf decompressedSize = compressed.readLU32();
        MemoryBuffer decompressed(decompressedSize);

        int result = uncompress(
                static_cast<unsigned char*>(decompressed.address()), &decompressedSize,
                static_cast<const unsigned char*>(compressed.currentPtr()), compressed.bytesLeft());
        if (result != Z_OK)
            return nullptr;
        if (decompressedSize != (uLongf)decompressed.size())
            return nullptr;

        Deserializer deserializer(decompressed.address(), decompressed.size());
        return load_1_5(deserializer);
    }

    // Unsupported version
    return nullptr;
}
FrameData* load_1_5(Deserializer& data)
{
    PROFILE(FrameDataGlobal, load_1_5);

    FrameIndex::Type frameCount = data.readLU32();
    int fighterCount = data.readU8();

    auto frames = SmallVector<Vector<FighterState>, 2>::makeResized(fighterCount);
    for (int fighter = 0; fighter != fighterCount; ++fighter)
        for (FrameIndex::Type frame = 0; frame < frameCount; ++frame)
        {
            const auto timeStamp = TimeStamp::fromMillisSinceEpoch(data.readLU64());
            const auto framesLeft = FramesLeft::fromValue(data.readLU32());
            const float posx = data.readLF32();
            const float posy = data.readLF32();
            const float damage = data.readLF32();
            const float hitstun = data.readLF32();
            const float shield = data.readLF32();
            const auto status = FighterStatus::fromValue(data.readLU16());
            const uint32_t motion_l = data.readLU32();
            const uint8_t motion_h = data.readU8();
            const auto motion = FighterMotion::fromParts(motion_h, motion_l);
            const auto hitStatus = FighterHitStatus::fromValue(data.readU8());
            const auto stocks = FighterStocks::fromValue(data.readU8());
            const auto flags = FighterFlags::fromValue(data.readU8());

            frames[fighter].push(FighterState(
                timeStamp,
                FrameIndex::fromValue(frame),
                framesLeft,
                Vec2::fromValues(posx, posy),
                damage,
                hitstun,
                shield,
                status,
                motion,
                hitStatus,
                stocks,
                flags));
        }

    return new FrameData(std::move(frames));
}

// ----------------------------------------------------------------------------
uint32_t FrameData::save(FILE* fp) const
{
    PROFILE(FrameData, save);

    const int fighterStateSize =
            sizeof(TimeStamp::Type) +
            sizeof(FramesLeft::Type) +
            sizeof(Vec2::ComponentType) * 2 +
            sizeof(fighters_[0][0].damage()) +
            sizeof(fighters_[0][0].hitstun()) +
            sizeof(fighters_[0][0].shield()) +
            sizeof(FighterStatus::Type) +
            5 + // motion is a hash40 (40 bits)
            sizeof(FighterHitStatus::Type) +
            sizeof(FighterStocks::Type) +
            sizeof(FighterFlags::Type);
    const int frameSize = fighterStateSize * fighterCount();

    MemoryBuffer uncompressed(5 + frameSize * frameCount());
    Serializer serializer(uncompressed.address(), uncompressed.size());

    serializer.writeLU32(frameCount());
    serializer.writeU8(fighterCount());
    for (const auto& fighter : fighters_)
        for (const auto& frame : fighter)
        {
            serializer.writeLU64(frame.timeStamp().millisSinceEpoch());
            serializer.writeLU32(frame.framesLeft().count());
            serializer.writeLF32(frame.pos().x());
            serializer.writeLF32(frame.pos().y());
            serializer.writeLF32(frame.damage());
            serializer.writeLF32(frame.hitstun());
            serializer.writeLF32(frame.shield());
            serializer.writeLU16(frame.status().value());
            serializer.writeLU32(frame.motion().lower());
            serializer.writeU8(frame.motion().upper());
            serializer.writeU8(frame.hitStatus().value());
            serializer.writeU8(frame.stocks().count());
            serializer.writeU8(frame.flags().value());
        }
    assert(serializer.bytesWritten() == (uint64_t)uncompressed.size());

    uLongf compressedSize = compressBound(uncompressed.size());
    MemoryBuffer compressed(compressedSize + 6);
    Serializer compressedSerializer(compressed.address(), compressed.size());
    compressedSerializer.writeU8(1);  // Major version
    compressedSerializer.writeU8(5);  // Minor version
    compressedSerializer.writeLU32(uncompressed.size());  // Decompressed size
    if (compress2(
            static_cast<uint8_t*>(compressedSerializer.writeToPtr(0)), &compressedSize,
            static_cast<const uint8_t*>(uncompressed.address()), uncompressed.size(),
            9) != Z_OK)
    {
        return 0;
    }
    compressedSerializer.writeToPtr(compressedSize);  // Move write pointer to the correct location so bytesWritten() is correct

    if (fwrite(compressed.address(), 1, compressedSerializer.bytesWritten(), fp) != compressedSerializer.bytesWritten())
        return 0;

    return compressedSerializer.bytesWritten();
}

// ----------------------------------------------------------------------------
int FrameData::fighterCount() const
{
    NOPROFILE();

    return fighters_.count();
}

// ----------------------------------------------------------------------------
int FrameData::frameCount() const
{
    NOPROFILE();

    return fighters_.count() ? fighters_[0].count() : 0;
}

// ----------------------------------------------------------------------------
const FighterState& FrameData::stateAt(int fighterIdx, int frameIdx) const
{
    NOPROFILE();

    assert(fighterIdx < fighterCount());
    assert(frameIdx < frameCount());
    return fighters_[fighterIdx][frameIdx];
}

// ----------------------------------------------------------------------------
const FighterState& FrameData::firstState(int fighterIdx) const
{
    NOPROFILE();

    assert(frameCount() > 0);
    return fighters_[fighterIdx].front();
}

// ----------------------------------------------------------------------------
const FighterState& FrameData::lastState(int fighterIdx) const
{
    NOPROFILE();

    assert(frameCount() > 0);
    return fighters_[fighterIdx].back();
}

// ----------------------------------------------------------------------------
void FrameData::addFrame(Frame<4>&& frame)
{
    PROFILE(FrameData, addFrame);

    // Sanity checks
    assert(frame.count() == fighterCount());
#ifndef NDEBUG
    for (int i = 1; i < fighterCount(); ++i)
    {
        assert(frame[0].framesLeft() == frame[i].framesLeft());
        assert(frame[0].frameIndex() == frame[i].frameIndex());
    }
#endif

    for (int i = 0; i < fighterCount(); ++i)
        fighters_[i].push(frame[i]);

    // If any fighter state is different from the previous one, notify
    if (frameCount() < 2 || framesHaveSameData(frameCount() - 1, frameCount() - 2) == false)
        dispatcher.dispatch(&FrameDataListener::onFrameDataNewUniqueFrame, frameCount() - 1, frame);

    // The UI cares about every frame
    dispatcher.dispatch(&FrameDataListener::onFrameDataNewFrame, frameCount() - 1, frame);
}

// ----------------------------------------------------------------------------
bool FrameData::framesHaveSameData(int frameIdx1, int frameIdx2) const
{
    PROFILE(FrameData, framesHaveSameData);

    // Compare each fighter state between each frame. If they are all equal,
    // then it means both frames compare equal
    for (int i = 0; i != fighterCount(); ++i)
        if (fighters_[i][frameIdx1].hasSameDataAs(fighters_[i][frameIdx2]) == false)
            return false;
    return true;
}

}
