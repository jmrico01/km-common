#pragma once

#include "km_lib.h"
#include "km_string.h"

template <typename Allocator>
Array<uint8> LoadEntireFile(const Array<char>& filePath, Allocator* allocator);
template <typename Allocator>
void FreeFile(const Array<uint8>& outFile, Allocator* allocator);

bool WriteFile(const Array<char>& filePath, const Array<uint8>& data, bool append);

bool DeleteFile(const Array<char>& filePath, bool errorIfNotFound);

bool CreateDirRecursive(const Array<char>& dir);

template <typename Allocator>
FixedArray<char, PATH_MAX_LENGTH> GetExecutablePath(Allocator* allocator);

bool RunCommand(const Array<char>& command);
