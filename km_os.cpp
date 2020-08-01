#include "km_os.h"

#if GAME_WIN32
#include <Windows.h>
#undef ERROR
#elif GAME_LINUX || GAME_MACOS
#include <stdlib.h>
#include <sys/stat.h>
#endif

#if GAME_WIN32
internal FILETIME Win32GetLastWriteTime(const char* filePath)
{
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesEx(filePath, GetFileExInfoStandard, &data)) {
        LOG_ERROR("GetFileAttributesEx failed for file %s\n", filePath);
        FILETIME zero = {};
        return zero;
    }
    return data.ftLastWriteTime;
}
#endif

template <typename Allocator>
Array<uint8> LoadEntireFile(const_string filePath, Allocator* allocator)
{
    Array<uint8> file = { .size = 0, .data = nullptr };
    char* filePathC = ToCString(filePath, allocator);
    if (!filePathC) {
        return { .size = 0, .data = nullptr };
    }

#if GAME_WIN32
    HANDLE hFile = CreateFile(filePathC, GENERIC_READ, FILE_SHARE_READ,
                              NULL, OPEN_EXISTING, NULL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return file;
    }
    allocator->Free(filePathC); // NOTE this has to be here to work with linear allocators... :(

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
#elif GAME_LINUX || GAME_MACOS
    FILE* filePtr = fopen(filePathC, "rb");
    if (filePtr == NULL) {
        return file;
    }
    allocator->Free(filePathC); // NOTE this has to be here to work with linear allocators... :(
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

bool WriteFile(const_string filePath, const Array<uint8>& data, bool append)
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
#elif GAME_LINUX || GAME_MACOS
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

bool DeleteFile(const_string filePath, bool errorIfNotFound)
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
#elif GAME_LINUX || GAME_MACOS
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

bool FileExists(const_string filePath)
{
    char* cFilePath = ToCString(filePath, &defaultAllocator_);
    defer(defaultAllocator_.Free(cFilePath));

#if GAME_WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE fileHandle = FindFirstFile(cFilePath, &findFileData);
    bool found = fileHandle != INVALID_HANDLE_VALUE;
    if (found) {
        FindClose(fileHandle);
    }
    return found;
#elif GAME_LINUX || GAME_MACOS
    // TODO implement
    return false;
#else
#error "FileExists not implemented on this platform"
#endif
}

bool FileChangedSinceLastCall(const_string filePath)
{
    char* cFilePath = ToCString(filePath, &defaultAllocator_);
    defer(defaultAllocator_.Free(cFilePath));

#if GAME_WIN32
    static HashTable<FILETIME> fileTimes;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
    }

    FILETIME lastWriteTime = Win32GetLastWriteTime(cFilePath);

    HashKey key(filePath);
    FILETIME* value = fileTimes.GetValue(key);
    if (!value) {
        fileTimes.Add(key, lastWriteTime);
        return true;
    }

    if (lastWriteTime.dwLowDateTime != value->dwLowDateTime
        || lastWriteTime.dwHighDateTime != value->dwHighDateTime) {
        *value = lastWriteTime;
        return true;
    }

    return false;
#elif GAME_LINUX || GAME_MACOS
    // TODO implement
    return false;
#else
#error "FileChangedSinceLastCall not implemented on this platform"
#endif
}

bool CreateDirRecursive(const_string dir)
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
#elif GAME_LINUX || GAME_MACOS
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
Array<string> ListDir(const_string dir, Allocator* allocator)
{
#if GAME_WIN32
    DynamicArray<string, Allocator> result(allocator);

    const_string starNull = {
        .size = 3,
        .data = "/*\0",
    };
    const_string dirStar = StringConcatenate(dir, starNull, allocator);
    if (dirStar.data == nullptr) {
        return Array<string>::empty;
    }

    WIN32_FIND_DATAA findData;
    HANDLE handle = FindFirstFileA(dirStar.data, &findData);
    if (handle == INVALID_HANDLE_VALUE) {
        return Array<string>::empty;
    }
    defer(FindClose(handle));

    // List all the files in the directory with some info about them
    do {
        string* path = result.Append();
        path->size = StringLength(findData.cFileName);
        path->data = allocator->New<char>(path->size);
        MemCopy(path->data, findData.cFileName, path->size);
    } while (FindNextFileA(handle, &findData) != 0);

    DWORD error = GetLastError();
    if (error != ERROR_NO_MORE_FILES) {
        return Array<string>::empty;
    }

    return result.ToArray();
#else
#error "ListDir not implemented on this platform"
#endif
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
#elif GAME_LINUX || GAME_MACOS
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

bool RunCommand(const_string command)
{
    char* commandCString = ToCString(command, &defaultAllocator_);

#if GAME_WIN32 || GAME_LINUX || GAME_MACOS
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
