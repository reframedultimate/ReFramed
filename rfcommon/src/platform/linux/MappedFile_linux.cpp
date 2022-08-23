#include "rfcommon/MappedFile.hpp"
#include "rfcommon/Profiler.hpp"

#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

namespace rfcommon {

// ----------------------------------------------------------------------------
MappedFile::MappedFile()
    : address_(nullptr)
{
}

// ----------------------------------------------------------------------------
MappedFile::~MappedFile()
{
    close();
}

// ----------------------------------------------------------------------------
bool MappedFile::open(const char* fileName)
{
    PROFILE(MappedFile, open);

    int fd;
    struct stat64 stbuf;
    void* address;

    // Try to open file. From what I understand, open64() is equivalent to
    // open() with the O_LARGEFILE flag set.
    if ((fd = ::open64(fileName, O_RDONLY)) < 0)
        goto open_failed;

    // Stat the file and ensure it is a regular file. st_size is only valid
    // for regular files, which we require in order to know the total size
    // of the file
    if (fstat64(fd, &stbuf) != 0)
        goto fstat_failed;
    if (!S_ISREG(stbuf.st_mode))
        goto fstat_failed;
    size_ = stbuf.st_size;

    // Map file into memory. We use MAP_PRIVATE since the file will always
    // be opened in read-only mode and thus we don't need to worry about
    // propagating changes to the file to other processes mapping the same
    // region.
    address = mmap(nullptr, stbuf.st_size, PROT_READ, MAP_PRIVATE | MAP_NORESERVE, fd, 0);
    if (address == MAP_FAILED)
        goto mmap_failed;

    // The file descriptor is no longer required and can be closed.
    ::close(fd);

    // Success, close previous mapping if any, then store new values
    close();
    address_ = address;
    size_ = stbuf.st_size;

    return true;

    mmap_failed  :
    fstat_failed : ::close(fd);
    open_failed  : return false;
}

// ----------------------------------------------------------------------------
bool MappedFile::setDeleteOnClose(const char* fileName)
{
    return unlink(fileName) == 0;
}

// ----------------------------------------------------------------------------
void MappedFile::close()
{
    PROFILE(MappedFile, close);

    if (address_)
        munmap(address_, size_);
    address_ = nullptr;
}

}
