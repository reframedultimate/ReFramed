#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/HighresTimer.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/RefCounted.hpp"

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <cstdio>
#include <cstdint>

#if defined(RFCOMMON_PROFILER)

namespace rfcommon {

class RFCOMMON_PUBLIC_API ProfileBlock : public RefCounted
{
public:
    typedef std::vector<Reference<ProfileBlock> > ContainerType;

    ProfileBlock(const char* name);
    ~ProfileBlock();

    void begin();
    void end();
    ProfileBlock* getChild(const char* name);

    ProfileBlock* parent;
    std::string name;
    ContainerType children;

    int executions() const;
    uint64_t averageTimeNS() const;
    uint64_t accumulatedTimeNS() const;
    uint64_t maxTimeNS() const;
    uint64_t minTimeNS() const;

private:
    int executions_;
    uint64_t accumulated_;
    uint64_t max_;
    uint64_t min_;
    HighresTimer timer_;
};


class RFCOMMON_PUBLIC_API ProfileRootBlock : public RefCounted
{
public:
    ProfileRootBlock(const char* name);

    void beginBlock(const char* blockName);
    void endBlock();
    ProfileBlock* getRoot();

private:
    ProfileBlock* activeBlock_;
    Reference<ProfileBlock> root_;
};


class RFCOMMON_PUBLIC_API Profiler
{
public:
    typedef std::map<uint32_t, Reference<ProfileRootBlock> > ContainerType;

    static void init();
    static void deinit();

    enum ExportType
    {
        DOT,
    };

    void beginBlock(const char* blockName);
    void endBlock();

    ProfileRootBlock* getThreadSpecificRootBlock();

    void exportGraph(FILE* stream, ExportType exportType);

    void clearAllData();

private:
    void exportGraph_DOT(FILE* stream);

    ContainerType rootBlocks_;
    std::mutex mutex_;
};


class RFCOMMON_PUBLIC_API AutoProfileBlock
{
public:
    AutoProfileBlock(ProfileRootBlock* profiler, const char* name) :
        profiler_(profiler)
    {
        profiler_->beginBlock(name);
    }

    ~AutoProfileBlock()
    {
        profiler_->endBlock();
    }

private:
    ProfileRootBlock* profiler_;
};

} // namespace rfcommon

namespace rfcommon {
extern RFCOMMON_PUBLIC_API Profiler* profiler;
}
#define PROFILE(classname, method) \
    ::rfcommon::AutoProfileBlock profile_##classname##_##method (::rfcommon::profiler->getThreadSpecificRootBlock(), #classname "::" #method)

#define PROFILE_BEGIN(name) ::rfcommon::profiler->beginBlock(name)
#define PROFILE_END()       ::rfcommon::profiler->endBlock()
#else
#define PROFILE(classname, method)
#define PROFILE_BEGIN(name)
#define PROFILE_END()
#endif

/// Use this so the script doesn't generate PROFILE() statements for a function
#define NOPROFILE()
