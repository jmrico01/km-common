#pragma once

#include "km_lib.h"
#include "km_string.h"

enum class FileType
{
    FILE,
    DIR
};

struct FileInfo
{
    string name;
    FileType type;
};

template <typename Allocator>
Array<uint8> LoadEntireFile(const_string filePath, Allocator* allocator);
template <typename Allocator>
void FreeFile(const Array<uint8>& outFile, Allocator* allocator);

bool WriteFile(const_string filePath, const Array<uint8>& data, bool append);

bool DeleteFile(const_string filePath, bool errorIfNotFound);

bool FileExists(const_string filePath);
bool FileChangedSinceLastCall(const_string filePath);

bool CreateDirRecursive(const_string dir);

// Traverses all files in the given directory. Returns an array with their names.
template <typename Allocator>
Array<FileInfo> ListDir(const_string dir, Allocator* allocator);
template <typename Allocator>
void FreeListDir(Array<FileInfo> fileInfos, Allocator* allocator);

template <typename Allocator>
FixedArray<char, PATH_MAX_LENGTH> GetExecutablePath(Allocator* allocator);

bool RunCommand(const_string command);
