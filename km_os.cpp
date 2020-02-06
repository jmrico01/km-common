#include "km_os.h"

#if GAME_WIN32
#include <Windows.h>
#elif GAME_LINUX
#include <stdlib.h>
#include <sys/stat.h>
#endif

template <typename Allocator>
Array<uint8> LoadEntireFile(const Array<char>& filePath, Allocator* allocator)
{
	Array<uint8> file;
	file.data = nullptr;
	char* cFilePath = ToCString(filePath, allocator);
	defer(allocator->Free(cFilePath));

#if GAME_WIN32
	HANDLE hFile = CreateFile(cFilePath, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return file;
	}

	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(hFile, &fileSize)) {
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
#elif GAME_LINUX
	FILE* filePtr = fopen(cFilePath, "rb");
	if (filePtr == NULL) {
		return file;
	}
	fseek(filePtr, 0, SEEK_END);
	uint64 size = ftell(filePtr);
	rewind(filePtr);

	file.data = (uint8*)allocator->Allocate(size);
	if (file.data == nullptr) {
		return file;
	}

	if (fread(file.data, size, 1, filePtr) != 1) {
		allocator->Free(file.data);
		file.data = nullptr;
		return file;
	}

	file.size = size;
	fclose(filePtr);
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

bool WriteFile(const Array<char>& filePath, const Array<uint8>& data, bool append)
{
	char* cFilePath = ToCString(filePath, &defaultAllocator_);
	defer(defaultAllocator_.Free(cFilePath));

#if GAME_WIN32
	HANDLE hFile = CreateFile(cFilePath, GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}

	if (append) {
		DWORD dwPos = SetFilePointer(hFile, 0, NULL, FILE_END);
		if (dwPos == INVALID_SET_FILE_POINTER) {
			return false;
		}
	}
	else {
		DWORD dwPos = SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		if (dwPos == INVALID_SET_FILE_POINTER) {
			return false;
		}
		if (SetEndOfFile(hFile) == 0) {
			return false;
		}
	}

	DWORD bytesWritten;
	if (!WriteFile(hFile, data.data, (DWORD)data.size, &bytesWritten, NULL)) {
		return false;
	}

	CloseHandle(hFile);
	return bytesWritten == (DWORD)data.size;
#elif GAME_LINUX
	FILE* filePtr;
	if (append) {
		filePtr = fopen(cFilePath, "ab");
	}
	else {
		filePtr = fopen(cFilePath, "w");
	}

	size_t written = fwrite(data.data, data.size, 1, filePtr);
	fclose(filePtr);

	return written == 1;
#else
#error "WriteFile not implemented on this platform"
#endif
}

bool DeleteFile(const Array<char>& filePath, bool errorIfNotFound)
{
	char* cFilePath = ToCString(filePath, &defaultAllocator_);
	defer(defaultAllocator_.Free(cFilePath));

#if GAME_WIN32
	BOOL result = DeleteFileA(cFilePath);
	if (result == 0) {
		DWORD error = GetLastError();
		if (error != ERROR_FILE_NOT_FOUND) {
			return false;
		}
		if (errorIfNotFound) {
			return false;
		}
	}
#elif GAME_LINUX
	int result = unlink(cFilePath);
	if (result == -1) {
		if (errno != ENOENT) {
			return false;
		}
		if (errorIfNotFound) {
			return false;
		}
	}
#else
#error "DeleteFile not implemented on this platform"
#endif

	return true;
}

bool CreateDirRecursive(const Array<char>& dir)
{
	FixedArray<char, PATH_MAX_LENGTH> path;
	uint64 nextSlash = 0;
	while (true) {
		nextSlash = dir.FindFirst('/', nextSlash + 1);
		if (nextSlash == dir.size) {
			break;
		}
		path.Clear();
		path.Append(dir.SliceTo(nextSlash));
		path.Append('\0');
#if GAME_WIN32
		BOOL result = CreateDirectoryA(path.data, NULL);
		if (result == 0) {
			DWORD error = GetLastError();
			if (error != ERROR_ALREADY_EXISTS) {
				return false;
			}
		}
#elif GAME_LINUX
		int result = mkdir(path.data, ACCESSPERMS);
		if (result == -1) {
			if (errno != EEXIST) {
				return false;
			}
		}
#else
#error "CreateDirRecursive not implemented on this platform"
#endif
	}

	return true;
}

template <typename Allocator>
FixedArray<char, PATH_MAX_LENGTH> GetExecutablePath(Allocator* allocator)
{
	FixedArray<char, PATH_MAX_LENGTH> path;
	path.Clear();

#if GAME_WIN32
	DWORD size = GetModuleFileName(NULL, path.data, PATH_MAX_LENGTH);
	if (size == 0) {
		return path;
	}
	path.size = size;
	for (uint64 i = 0; i < path.size; i++) {
		if (path[i] == '\\') {
			path[i] = '/';
		}
	}
#elif GAME_LINUX
	ssize_t count = readlink("/proc/self/exe", path.data, PATH_MAX_LENGTH);
	if (count == -1) {
		return path;
	}
	path.size = count;
#else
#error "GetExecutablePath not implemented on this platform"
#endif

	path.size = path.ToArray().FindLast('/') + 1;
	return path;
}

bool RunCommand(const Array<char>& command)
{
	char* commandCString = ToCString(command, &defaultAllocator_);

#if GAME_WIN32 || GAME_LINUX
	int result = system(commandCString);
	if (result != 0) {
		LOG_ERROR("RunCommand system(...) call returned %d\n", result);
		return false;
	}
#else
#error "GetExecutablePath not implemented on this platform"
#endif

	return true;
}
