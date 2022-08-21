#include "rfcommon/MappedFile.hpp"
#include "rfcommon/Profiler.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

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
    if (hFile == INVALID_HANDLE_VALUE)
        goto open_failed;

    // Determine file size in bytes
    if (!GetFileSizeEx(hFile, &liFileSize))
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
}

// ----------------------------------------------------------------------------
void MappedFile::close()
{
    PROFILE(MappedFile, close);

    if (address_)
        UnmapViewOfFile(address_);
    address_ = nullptr;
}

}
