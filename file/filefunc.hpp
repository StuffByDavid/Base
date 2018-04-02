#pragma once

#ifdef _WIN32
    #define SLASH L"\\"
#else
    #define SLASH L"/"
#endif

#define DOT L"."

#include <fstream>
#include <sys/stat.h>

#include "util/stringfunc.hpp"


namespace Base
{
    /* Represents a file stored in memory. */
    struct File
    {
        string name;
        void* loaded;
        char* rawData;
        size_t size;
    };

    /* Returns whether the given file exists. */
    bool fileExists(wstring filename);

    /* Returns the size of the given file, in bytes. */
    int fileGetSize(wstring filename);

    /* Returns the name of the file, without the path. */
    wstring fileGetName(wstring filename);

    /* Returns the path to the given file, including the final slash. */
    wstring fileGetPath(wstring filename);

    /* Returns the directory of the given file, not including the final slash. */
    wstring fileGetDirectory(wstring filename);

    /* Returns the extension of the file, including the leading dot. */
    wstring fileGetExtension(wstring filename);

    /* Returns the filename with the new extension (extension must include the leading dot). */
    wstring fileSetExtension(wstring filename, wstring ext);

    /* Returns the contents of the given text file. */
    wstring fileGetContents(wstring filename);

    /* Returns whether the given directory exists. */
    bool directoryExists(wstring directory);
}