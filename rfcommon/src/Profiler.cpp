#include "common/Profiler.hpp"
#include "common/Thread.hpp"
#include "common/Logger.hpp"

#include <limits>
#include <stdio.h>


namespace common {

Profiler g_profiler;

// ----------------------------------------------------------------------------
ProfileBlock::ProfileBlock(const char* blockName) :
    parent(NULL),
    name(blockName),
    executions_(0),
    accumulated_(0),
    max_(0),
    min_(std::numeric_limits<double>::max())
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

    double passed = double(timer_.getTimePassedInNanoSeconds()) / 1000000000.0;
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
int ProfileBlock::getExecutions() const
{
    NOPROFILE();

    return executions_;
}

// ----------------------------------------------------------------------------
ParameterD ProfileBlock::getAverageTime() const
{
    NOPROFILE();

    if(executions_ == 0)
        return ParameterD(0.0, Metric::SECONDS);

    return ParameterD(accumulated_ / executions_, Metric::SECONDS);
}

// ----------------------------------------------------------------------------
ParameterD ProfileBlock::getAccumulatedTime() const
{
    NOPROFILE();

    if(executions_ == 0)
        return ParameterD(0.0, Metric::SECONDS);

    return ParameterD(accumulated_, Metric::SECONDS);
}

// ----------------------------------------------------------------------------
ParameterD ProfileBlock::getMaxTime() const
{
    NOPROFILE();

    if(executions_ == 0)
        return ParameterD(0.0, Metric::SECONDS);

    return ParameterD(max_, Metric::SECONDS);
}

// ----------------------------------------------------------------------------
ParameterD ProfileBlock::getMinTime() const
{
    NOPROFILE();

    if(executions_ == 0)
        return ParameterD(0.0, Metric::SECONDS);

    return ParameterD(min_, Metric::SECONDS);
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

    Lock lock(&mutex_);

    uint32_t tid = Thread::getTID();
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

    Lock lock(&mutex_);

    switch(exportType)
    {
        case TEXT_GRAPH:       exportGraph_Text(stream);       break;
        case TEXT_HIGHEST_ACC: exportGraph_HighestAcc(stream); break;
        case XML:              g_defaultLogger.logError("Not implemented. Use a common::File* instead."); break;
        case DOT:              g_defaultLogger.logError("Not implemented. Use a common::File* instead."); break;
        case CODE_FLOWER:      g_defaultLogger.logError("Not implemented. Use a common::File* instead."); break;
    };
}

// ----------------------------------------------------------------------------
void Profiler::exportGraph(File* file, Profiler::ExportType exportType)
{
    NOPROFILE();

    Lock lock(&mutex_);

    switch(exportType)
    {
        case TEXT_GRAPH:       g_defaultLogger.logError("Not implemented. Use a FILE* instead."); break;
        case TEXT_HIGHEST_ACC: g_defaultLogger.logError("Not implemented. Use a FILE* instead."); break;
        case XML:              exportGraph_XML(file);        break;
        case DOT:              exportGraph_DOT(file);        break;
        case CODE_FLOWER:      exportGraph_CodeFlower(file); break;
    };
}

// ----------------------------------------------------------------------------
void Profiler::clearAllData()
{
    NOPROFILE();

    rootBlocks_.clear();
}

}
