#pragma once

#include "rfcommon/String.hpp"

namespace rfcommon {

class VideoFileResolver
{
public:
    /*!
     * \brief Search for the file name in all video source paths and return
     * an absolute path + filename to the file. If the file cannot be found,
     * return an empty string.
     */
    virtual String resolveVideoFile(const char* fileName) const = 0;
};

}
