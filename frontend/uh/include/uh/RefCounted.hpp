#pragma once

#include "uh/config.hpp"

namespace uh {

class UH_PUBLIC_API RefCounted
{
public:
	RefCounted();
	virtual ~RefCounted();

	RefCounted(const RefCounted&) = delete;
	RefCounted& operator=(const RefCounted&) = delete;

	void incRef();
	void decRef();

	int refs() const { return refs_; }

protected:
	virtual void seppuku();

private:
	int refs_;
};

}
