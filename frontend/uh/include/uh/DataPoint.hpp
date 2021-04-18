#pragma once

#include "uh/config.hpp"
#include <cstdint>

namespace uh {

class Recording;
class Video;

class UH_PUBLIC_API DataPoint
{
public:

private:
	Recording* recording_;
	Video* video_;
	uint64_t absoluteTime_;
	uint32_t frame_;

};

}
