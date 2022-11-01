#include "rfcommon/Profiler.hpp"

#include <algorithm>
#include <limits>
#include <set>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace rfcommon {

// ----------------------------------------------------------------------------
static std::string nsToEng(uint64_t ns)
{
    NOPROFILE();

    static const char* table[] = { "ns", "us", "ms", "s" };
    int i = 0;
    double result = ns;
    while (result > 1000 && i < 3)
    {
        result /= 1000;
        i++;
    }

    std::ostringstream ss;
    ss << std::setprecision(3) << result << table[i];
    return ss.str();
}

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
static void exportGraph_DOTRecursive(const ProfileBlock* block, FILE* fp, double min, double max)
{
    NOPROFILE();

    std::string nodeName = block->name;

    for(ProfileBlock::ContainerType::const_iterator it = block->children.begin(); it != block->children.end(); ++it)
    {
        ProfileBlock* childBlock = *it;
        std::string childName = removeIllegalDOTChars(childBlock->name);
        std::string dotSyntax = "\t" + nodeName + " -> " + childName + " [len=5];\n";
        fprintf(fp, dotSyntax.c_str());

        exportGraph_DOTRecursive(childBlock, fp, min, max);
    }

    // Calculate colour depending on the accumulated time of the block
    std::string colorAttribute;
    {
        double acc = block->accumulatedTimeNS() / 1e9;
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
           << "Executions: " << block->executions() << " | "
           << "Accumulated: " << nsToEng(block->accumulatedTimeNS()) << " |{ "
           << "min: " << nsToEng(block->minTimeNS()) << " | "
           << "avg: " << nsToEng(block->averageTimeNS()) << " | "
           << "max: " << nsToEng(block->maxTimeNS()) << "} }\"";
        labelAttribute = ss.str();
    }

    std::string dotSyntax = "\t" + nodeName + " [shape=record," + colorAttribute + "," + labelAttribute + "];\n";
    fprintf(fp, dotSyntax.c_str());
}

// ----------------------------------------------------------------------------
static void determineMinMaxAcc(const ProfileBlock* block, double* min, double* max)
{
    NOPROFILE();

    double acc = block->accumulatedTimeNS() / 1e9;
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
void Profiler::exportGraph_DOT(FILE* fp)
{
    NOPROFILE();

    fprintf(fp, "digraph graphname {\n");
    for(ContainerType::const_iterator it = rootBlocks_.begin(); it != rootBlocks_.end(); ++it)
    {
        ProfileBlock* rootBlock = it->second->getRoot();
        double min = std::numeric_limits<double>::max();
        double max = -std::numeric_limits<double>::max();
        determineMinMaxAcc(rootBlock, &min, &max);
        std::set<std::string> uniqueNames;
        makeProfileBlockNamesUnique(rootBlock, &uniqueNames);

        exportGraph_DOTRecursive(rootBlock, fp, min, max);
    }
    fprintf(fp, "}\n");
}

}
