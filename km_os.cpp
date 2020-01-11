#include "km_os.h"

#if GAME_WIN32
#include <Windows.h>
#endif

template <typename Allocator>
Array<uint8> LoadEntireFile(const Array<char>& filePath, Allocator* allocator)
{
	Array<uint8> file;
	char* cFilePath = ToCString(filePath, allocator);

#if GAME_WIN32
	HANDLE hFile = CreateFile(cFilePath, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, NULL, NULL);
	allocator->Free(cFilePath);
	if (hFile == INVALID_HANDLE_VALUE) {
		file.data = nullptr;
		return file;
	}

	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(hFile, &fileSize)) {
		file.data = nullptr;
		return file;
	}

	uint32 fileSize32 = SafeTruncateUInt64(fileSize.QuadPart);
	file.data = (uint8*)allocator->Allocate(fileSize32);
	if (file.data == nullptr) {
		return file;
	}

	DWORD bytesRead;
	if (!ReadFile(hFile, file.data, fileSize32, &bytesRead, NULL) || fileSize32 != bytesRead) {
		allocator->Free(file.data);
		file.data = nullptr;
		return file;
	}

	file.size = fileSize32;
	CloseHandle(hFile);
#else
	#error "LoadEntireFile not implemented on this platform"
#endif

	return file;
}

template <typename Allocator>
void FreeFile(const Array<uint8>& outFile, Allocator* allocator)
{
	allocator->Free(outFile.data);
}

template <typename Allocator>
FixedArray<char, PATH_MAX_LENGTH> GetExecutablePath(Allocator* allocator)
{
	FixedArray<char, PATH_MAX_LENGTH> path;
	path.size = 0;

#if GAME_WIN32
	DWORD size = GetModuleFileName(NULL, path.data, PATH_MAX_LENGTH);
	if (size == 0) {
		return path;
	}
	path.size = size;
	path.size = GetLastOccurrence(path.ToArray(), '\\');

	for (uint64 i = 0; i < path.size; i++) {
		if (path[i] == '\\') {
			path[i] = '/';
		}
	}
#else
	#error "GetExecutablePath not implemented on this platform"
#endif

	return path;
}
