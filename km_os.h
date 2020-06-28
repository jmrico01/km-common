#pragma once

#include "km_array.h"
#include "km_string.h"

template <typename Allocator>
Array<uint8> LoadEntireFile(const_string filePath, Allocator* allocator);
template <typename Allocator>
void FreeFile(Array<uint8> file, Allocator* allocator);

bool WriteFile(const_string filePath, const Array<uint8>& data, bool append);

bool DeleteFile(const_string filePath, bool errorIfNotFound);

bool FileExists(const_string filePath);
bool FileChangedSinceLastCall(const_string filePath);

bool CreateDirRecursive(const_string dir);

template <typename Allocator>
FixedArray<char, PATH_MAX_LENGTH> GetExecutablePath(Allocator* allocator);

bool RunCommand(const_string command);
