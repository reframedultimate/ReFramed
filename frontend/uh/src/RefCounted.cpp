#include "uh/RefCounted.hpp"
#include <cassert>

namespace uh {

// ----------------------------------------------------------------------------
RefCounted::RefCounted()
	: refs_(0)
{}

// ----------------------------------------------------------------------------
RefCounted::~RefCounted()
{
	assert(refs_ == 0);
	refs_ = -1;
}

// ----------------------------------------------------------------------------
void RefCounted::incRef()
{
	assert(refs_ >= 0);
	refs_++;
}

// ----------------------------------------------------------------------------
void RefCounted::decRef()
{
	assert(refs_ > 0);
	refs_--;
	if (refs_ == 0)
		seppuku();
}

// ----------------------------------------------------------------------------
void RefCounted::seppuku()
{
	delete this;
}

}
