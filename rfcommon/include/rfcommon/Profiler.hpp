#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/HighresTimer.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/DeltaTime.hpp"

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <cstdio>

namespace rfcommon {

class ProfileBlock : public RefCounted
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

    int getExecutions() const;
    uint64_t getAverageTime() const;
    uint64_t getAccumulatedTime() const;
    uint64_t getMaxTime() const;
    uint64_t getMinTime() const;

private:
    int executions_;
    double accumulated_;
    double max_;
    double min_;
    HighresTimer timer_;
};


class ProfileRootBlock : public RefCounted
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


class Profiler
{
public:
    typedef std::map<uint32_t, Reference<ProfileRootBlock> > ContainerType;

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


class AutoProfileBlock
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

} // namespace common

#ifdef RFCOMMON_PROFILER
namespace rfcommon {
extern Profiler profiler;
}
#define PROFILE(classname, method) \
    ::common::AutoProfileBlock profile_##classname##_##method (::common::profiler.getThreadSpecificRootBlock(), #classname "::" #method)

#define PROFILE_BEGIN(name) ::common::profiler.beginBlock(name)
#define PROFILE_END()       ::common::profiler.endBlock()
#else
#define PROFILE(classname, method)
#define PROFILE_BEGIN(name)
#define PROFILE_END()
#endif

/// Use this so the script doesn't generate PROFILE() statements for a function
#define NOPROFILE()
