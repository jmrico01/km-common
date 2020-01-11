#pragma once

#include "km_lib.h"
#include "km_string.h"

template <typename Allocator>
Array<uint8> LoadEntireFile(const Array<char>& filePath, Allocator* allocator);

template <typename Allocator>
void FreeFile(const Array<uint8>& outFile, Allocator* allocator);

template <typename Allocator>
FixedArray<char, PATH_MAX_LENGTH> GetExecutablePath(Allocator* allocator);
