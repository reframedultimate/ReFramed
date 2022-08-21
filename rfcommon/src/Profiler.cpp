#include "rfcommon/Profiler.hpp"
#include "rfcommon/Log.hpp"

#include <limits>

#if defined(RFCOMMON_PLATFORM_WINDOWS)
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#else
#   include <sys/syscall.h>
#endif

#if defined(max)
#   undef max
#endif

namespace rfcommon {

Profiler* profiler = nullptr;

// ----------------------------------------------------------------------------
ProfileBlock::ProfileBlock(const char* blockName) :
    parent(NULL),
    name(blockName),
    executions_(0),
    accumulated_(0),
    max_(0),
    min_(std::numeric_limits<uint64_t>::max())
{
}

// ----------------------------------------------------------------------------
ProfileBlock::~ProfileBlock()
{
}

// ----------------------------------------------------------------------------
void ProfileBlock::begin()
{
    NOPROFILE();

    timer_.start();
}

// ----------------------------------------------------------------------------
void ProfileBlock::end()
{
    NOPROFILE();

    timer_.stop();

    uint64_t passed = timer_.getTimePassedInNanoSeconds();
    accumulated_ += passed;
    executions_++;

    if(passed > max_)
        max_ = passed;

    if(passed < min_)
        min_ = passed;
}

// ----------------------------------------------------------------------------
ProfileBlock* ProfileBlock::getChild(const char* childName)
{
    NOPROFILE();

    // If child exists, return it
    for(ContainerType::iterator it = children.begin(); it != children.end(); ++it)
    {
        if((*it)->name == childName)
            return *it;
    }

    // Create new block
    children.push_back(new ProfileBlock(childName));
    children.back()->parent = this;
    return children.back();
}

// ----------------------------------------------------------------------------
int ProfileBlock::executions() const
{
    NOPROFILE();

    return executions_;
}

// ----------------------------------------------------------------------------
uint64_t ProfileBlock::averageTimeNS() const
{
    NOPROFILE();

    if (executions_ == 0)
        return 0;

    return accumulated_ / executions_;
}

// ----------------------------------------------------------------------------
uint64_t ProfileBlock::accumulatedTimeNS() const
{
    NOPROFILE();

    if (executions_ == 0)
        return 0;

    return accumulated_;
}

// ----------------------------------------------------------------------------
uint64_t ProfileBlock::maxTimeNS() const
{
    NOPROFILE();

    if (executions_ == 0)
        return 0;

    return max_;
}

// ----------------------------------------------------------------------------
uint64_t ProfileBlock::minTimeNS() const
{
    NOPROFILE();

    if (executions_ == 0)
        return 0;

    return min_;
}

// ----------------------------------------------------------------------------
ProfileRootBlock::ProfileRootBlock(const char* name) :
    root_(new ProfileBlock(name))
{
    activeBlock_ = root_;
}

// ----------------------------------------------------------------------------
void ProfileRootBlock::beginBlock(const char* blockName)
{
    NOPROFILE();

    activeBlock_ = activeBlock_->getChild(blockName);
    activeBlock_->begin();
}

// ----------------------------------------------------------------------------
void ProfileRootBlock::endBlock()
{
    NOPROFILE();

    activeBlock_->end();
    if(activeBlock_->parent)
        activeBlock_ = activeBlock_->parent;
}

// ----------------------------------------------------------------------------
ProfileBlock* ProfileRootBlock::getRoot()
{
    NOPROFILE();

    return root_;
}

// ----------------------------------------------------------------------------
void Profiler::init()
{
    if (profiler)
        return;
    profiler = new Profiler;
}

// ----------------------------------------------------------------------------
void Profiler::deinit()
{
    delete profiler;
    profiler = nullptr;
}

// ----------------------------------------------------------------------------
void Profiler::beginBlock(const char* blockName)
{
    NOPROFILE();

    getThreadSpecificRootBlock()->beginBlock(blockName);
}

// ----------------------------------------------------------------------------
void Profiler::endBlock()
{
    NOPROFILE();

    getThreadSpecificRootBlock()->endBlock();
}

// ----------------------------------------------------------------------------
ProfileRootBlock* Profiler::getThreadSpecificRootBlock()
{
    NOPROFILE();

    std::lock_guard<std::mutex> guard(mutex_);

#if defined(RFCOMMON_PLATFORM_WINDOWS)
    uint32_t tid = (uint32_t)GetCurrentThreadId();
#else
    uint32_t tid = (uint32_t)syscall(SYS_gettid);
#endif
    ContainerType::iterator it = rootBlocks_.find(tid);

    if(it != rootBlocks_.end())
        return it->second;

    char name[18];
    sprintf(name, "Thread 0x%x", tid);
    ProfileRootBlock* block = new ProfileRootBlock(name);
    rootBlocks_[tid] = block;
    return block;
}

// ----------------------------------------------------------------------------
void Profiler::exportGraph(FILE* stream, Profiler::ExportType exportType)
{
    NOPROFILE();

    std::lock_guard<std::mutex> guard(mutex_);

    switch(exportType)
    {
        case DOT: exportGraph_DOT(stream); break;
    };
}

// ----------------------------------------------------------------------------
void Profiler::clearAllData()
{
    NOPROFILE();

    rootBlocks_.clear();
}

}
