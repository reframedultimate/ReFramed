#include "common/Profiler.hpp"
#include "common/File.hpp"

#include <algorithm>
#include <limits>
#include <set>

namespace common {

// ----------------------------------------------------------------------------
static std::string removeIllegalDOTChars(const std::string& str)
{
    NOPROFILE();

    std::string ret = str;
    for(std::string::iterator it = ret.begin(); it != ret.end(); ++it)
    {
        if(!(isalpha(*it) || isdigit(*it)))
            *it = '_';
    }
    return ret;
}

// ----------------------------------------------------------------------------
static void makeProfileBlockNamesUnique(ProfileBlock* block, std::set<std::string>* uniqueNames)
{
    NOPROFILE();

    // Remove illegal characters for DOT format
    std::string name = removeIllegalDOTChars(block->name);

    // Name must be unique
    int suffix = 1;
    std::string proposedName = name;
    while(uniqueNames->find(proposedName) != uniqueNames->end())
    {
        std::ostringstream ss;
        ss << suffix++;
        proposedName = name + "_" + ss.str();
    }
    uniqueNames->insert(proposedName);
    block->name = proposedName;

    for(ProfileBlock::ContainerType::const_iterator it = block->children.begin(); it != block->children.end(); ++it)
    {
        makeProfileBlockNamesUnique(*it, uniqueNames);
    }
}

// ----------------------------------------------------------------------------
static void exportGraph_DOTRecursive(const ProfileBlock* block, File* file, double min, double max)
{
    NOPROFILE();

    std::string nodeName = block->name;

    for(ProfileBlock::ContainerType::const_iterator it = block->children.begin(); it != block->children.end(); ++it)
    {
        ProfileBlock* childBlock = *it;
        std::string childName = removeIllegalDOTChars(childBlock->name);
        std::string dotSyntax = "\t" + nodeName + " -> " + childName + " [len=5];\n";
        file->writeString(dotSyntax.c_str());

        exportGraph_DOTRecursive(childBlock, file, min, max);
    }

    // Calculate colour depending on the accumulated time of the block
    std::string colorAttribute;
    {
        double acc = block->getAccumulatedTime().toSI();
        double weight = (acc - min) / (max - min);   // scale to [0..1]
        if (weight > 0)
            weight = std::pow(weight, 0.1);          // better distribution
        weight = (2.0 / 3.0) - (weight * 2.0 / 3.0); // hue goes from 0 (red) to 0.666 (blue)
        std::ostringstream ss;
        ss << weight;
        colorAttribute = "color=\"" + ss.str() + " 1.0 1.0\"";
    }

    std::string labelAttribute;
    {
        std::ostringstream ss;
        ss << "label=\"{ " << nodeName << " | "
           << "Executions: " << block->getExecutions() << " | "
           << "Accumulated: " << block->getAccumulatedTime().toHumanReadable(3) << " |{ "
           << "min: " << block->getMinTime().toHumanReadable(3) << " | "
           << "avg: " << block->getAverageTime().toHumanReadable(3) << " | "
           << "max: " << block->getMaxTime().toHumanReadable(3) << "} }\"";
        labelAttribute = ss.str();
    }

    std::string dotSyntax = "\t" + nodeName + " [shape=record," + colorAttribute + "," + labelAttribute + "];\n";
    file->writeString(dotSyntax.c_str());
}

// ----------------------------------------------------------------------------
static void determineMinMaxAcc(const ProfileBlock* block, double* min, double* max)
{
    NOPROFILE();

    double acc = block->getAccumulatedTime().toSI();
    if(acc != 0.0)
    {
        if(acc < *min)
            *min = acc;
        if(acc > *max)
            *max = acc;
    }

    for(ProfileBlock::ContainerType::const_iterator it = block->children.begin(); it != block->children.end(); ++it)
    {
        determineMinMaxAcc(*it, min, max);
    }
}

// ----------------------------------------------------------------------------
void Profiler::exportGraph_DOT(File* file)
{
    NOPROFILE();

    for(ContainerType::const_iterator it = rootBlocks_.begin(); it != rootBlocks_.end(); ++it)
    {
        ProfileBlock* rootBlock = it->second->getRoot();
        double min = std::numeric_limits<double>::max();
        double max = -std::numeric_limits<double>::max();
        determineMinMaxAcc(rootBlock, &min, &max);
        std::set<std::string> uniqueNames;
        makeProfileBlockNamesUnique(rootBlock, &uniqueNames);

        file->writeString("digraph graphname {\n");
        exportGraph_DOTRecursive(rootBlock, file, min, max);
        file->writeString("}\n\n");
    }
}

}
