#include "rfcommon/MappedFile.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Utf8.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace rfcommon {

// ----------------------------------------------------------------------------
MappedFile::MappedFile()
    : fileHandle_(nullptr)
    , address_(nullptr)
{
}

// ----------------------------------------------------------------------------
MappedFile::~MappedFile()
{
    close();
}

// ----------------------------------------------------------------------------
bool MappedFile::open(const char* utf8_filename)
{
    PROFILE(MappedFile, open);

    HANDLE hFile;
    LARGE_INTEGER liFileSize;
    HANDLE mapping;
    void* address;
    wchar_t* utf16_filename;

    utf16_filename = utf8_to_utf16(utf8_filename, strlen(utf8_filename));
    if (utf16_filename == nullptr)
        goto utf16_conv_failed;

    // Try to open the file
    hFile = CreateFileW(
        utf16_filename,         // File name
        GENERIC_READ,           // Read only
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
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

    // The file mapping isn't required anymore
    CloseHandle(mapping);

    // Success, close previous mapping if any, then store new values
    close();
    fileHandle_ = static_cast<void*>(hFile);
    address_ = address;
    size_ = liFileSize.QuadPart;

    return true;

    map_view_failed            : CloseHandle(mapping);
    create_file_mapping_failed :
    get_file_size_failed       : CloseHandle(hFile);
    open_failed                : utf16_free(utf16_filename);
    utf16_conv_failed          : return false;
}

// ----------------------------------------------------------------------------
void MappedFile::close()
{
    PROFILE(MappedFile, close);

    if (address_)
        UnmapViewOfFile(address_);
    address_ = nullptr;

    if (fileHandle_)
        CloseHandle(static_cast<HANDLE>(fileHandle_));
    fileHandle_ = nullptr;
}

// ----------------------------------------------------------------------------
bool MappedFile::setDeleteOnClose(const char* utf8_filename)
{
    PROFILE(MappedFile, setDeleteOnClose);

    wchar_t* utf16_filename = utf8_to_utf16(utf8_filename, strlen(utf8_filename));
    if (utf16_filename == nullptr)
        return false;

    HANDLE hFile = CreateFileW(utf16_filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, NULL);
    utf16_free(utf16_filename);
    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    CloseHandle(hFile);
    return true;
}

}
