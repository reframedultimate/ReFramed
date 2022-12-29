#include "rfcommon/Profiler.hpp"
#include "application/widgets/CollapsibleSplitter.hpp"

namespace rfapp {

// ----------------------------------------------------------------------------
CollapsibleSplitter::CollapsibleSplitter(Qt::Orientation orientation)
    : QSplitter(orientation)
{}

// ----------------------------------------------------------------------------
CollapsibleSplitter::~CollapsibleSplitter()
{}

// ----------------------------------------------------------------------------
void CollapsibleSplitter::toggleCollapse()
{
    PROFILE(CollapsibleSplitter, toggleCollapse);

    auto s = sizes();
    if (s[0] == 0)
    {
        s[0] = store_;
        s[1] -= store_;
    }
    else
    {
        store_ = s[0];
        s[0] = 0;
        s[1] += store_;
    }
    setSizes(s);
}

}
