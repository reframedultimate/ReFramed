#include "rfcommon/MappedFile.hpp"

#if defined(_WIN32)
#   include <Windows.h>
#elif defined(__APPLE__)
#elif defined(__linux__)
#   include <unistd.h>
#   include <sys/fcntl.h>
#   include <sys/mman.h>
#   include <sys/stat.h>
#else
#   error "Platform not supported"
#endif

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
#if defined(_WIN32)
    HANDLE hFile;
    LARGE_INTEGER liFileSize;
    HANDLE mapping;
    void* address;

    // Try to open the file
    hFile = CreateFile(
        fileName,               // File name
        GENERIC_READ,           // Read only
        0,                      // We have exclusive access over file
        NULL,                   // Default security
        OPEN_EXISTING,          // File must exist
        FILE_ATTRIBUTE_NORMAL,  // Default attributes
        NULL);                  // No attribute template
    if (hFile == INVALID_FILE_HANDLE)
        goto open_failed;

    // Determine file size in bytes
    if (!GetFileSize(hFile, &liFileSize))
        goto get_file_size_failed;

    mapping = CreateFileMapping(
        hFile,                 // File handle
        NULL,                  // Default security attributes
        PAGE_READONLY,         // Read only (or copy on write, but we don't write)
        0, 0,                  // High/Low size of mapping. Zero means entire file
        NULL);                 // Don't name the mapping
    if (mapping == NULL)
        goto create_file_mapping_failed;

    address = MapViewOfFile(
        mapping,               // File mapping handle
        FILE_MAP_READ,         // Read-only view of file
        0, 0,                  // High/Low offset of where the mapping should begin in the file
        0);                    // Length of mapping. Zero means entire file
    if (address == NULL)
        goto map_view_failed;

    // The file handle and file mapping aren't required anymore
    CloseHandle(mapping);
    CloseHandle(hFile);

    // Success, close previous mapping if any, then store new values
    close();
    address_ = address;
    size_ = liFileSize.QuadPart;

    return true;

    map_view_failed            : CloseHandle(mapping);
    create_file_mapping_failed :
    get_file_size_failed       : CloseHandle(hFile);
    open_failed                : return false;
#elif defined(__linux__)
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
    address = mmap(nullptr, stbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
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
#endif
}

// ----------------------------------------------------------------------------
void MappedFile::close()
{
#if defined(_WIN32)
    if (address_)
        UnmapViewOfFile(address_);
    address_ = nullptr;
#elif defined(__linux__)
    if (address_)
        munmap(address_, size_);
    address_ = nullptr;
#endif
}

}
