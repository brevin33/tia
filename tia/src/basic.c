#include "tia.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

void red_printf(const char* message, ...) {
    printf("\x1b[31m");
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    printf("\x1b[0m");
}

void green_printf(const char* message, ...) {
    printf("\x1b[32m");
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    printf("\x1b[0m");
}

void* alloc(size_t size) {
    assert(size > 0);
    assert(size < 1024 * 1024);  // just doing this to know if a big allaocation happens as i tis probably a bug
    return arena_alloc(context.arena, size);
}

u64 get_string_uint(const char* string, bool* out_error) {
    errno = 0;
    char* endptr;
    unsigned long long value = strtoull(string, &endptr, 10);

    *out_error = (errno != 0 || endptr == string || *endptr != '\0');
    errno = 0;
    return (u64)value;
}

char* make_portable_slashes(const char* path) {
    u64 len = strlen(path);
    char* new_path = alloc(len + 1);
    for (u64 i = 0; i < len; i++) {
        if (path[i] == '\\') {
            new_path[i] = '/';
        } else {
            new_path[i] = path[i];
        }
    }
    new_path[len] = '\0';
    return new_path;
}

bool is_number_float(char* c) {
    while (*c == '\0') {
        if (*c == '.') {
            return true;
        }
    }
    return false;
}

double string_to_double(char* string) {
    const char* str = "3.14159";
    char* endptr;
    double value = strtod(str, &endptr);
    return value;
}

char* read_file(const char* path_) {
    char* path = make_portable_slashes(path_);
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* contents = alloc(size + 1);
    fread(contents, 1, size, file);
    contents[size] = '\0';
    fclose(file);
    return contents;
}

char* get_file_extension(const char* path_) {
    char* path = make_portable_slashes(path_);
    char* dot = strrchr(path, '.');
    if (dot == NULL) {
        return NULL;
    }
    return dot + 1;
}

char* get_file_name(const char* path_) {
    char* path = make_portable_slashes(path_);
    char* slash = strrchr(path, '/');
    if (slash == NULL) {
        char* copy = alloc(strlen(path) + 1);
        return copy;
    }
    return slash + 1;
}

char** get_file_in_directory(const char* path_, u64* out_count) {
    char* path = make_portable_slashes(path_);
    u64 file_count = 0;
    u64 files_capacity = 8;
    char** files = alloc(files_capacity * sizeof(char*));

#if defined(_WIN32) || defined(_WIN64)
    WIN32_FIND_DATA find_file_data = {0};
    char search_path[MAX_PATH];
    snprintf(search_path, MAX_PATH, "%s/*", path);
    HANDLE hFind = FindFirstFile(search_path, &find_file_data);

    if (hFind == INVALID_HANDLE_VALUE) {
        *out_count = 0;
        return NULL;
    }
    for (;;) {
        if (!(find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            if (file_count >= files_capacity) {
                files_capacity *= 2;
                void* old_files = files;
                files = alloc(files_capacity * sizeof(char*));
                memcpy(files, old_files, file_count * sizeof(char*));
            }
            size_t len = strlen(find_file_data.cFileName) + 1;
            files[file_count] = alloc(len);
            memcpy(files[file_count], find_file_data.cFileName, len);
            file_count++;
        }
        if (!FindNextFile(hFind, &find_file_data)) break;
    }
    FindClose(hFind);
#else
    DIR* dir = opendir(path);
    if (!dir) {
        *out_count = 0;
        return NULL;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_DIR) {
            if (file_count >= files_capacity) {
                files_capacity *= 2;
                void* old_files = files;
                files = alloc(files_capacity * sizeof(char*));
                memcpy(files, old_files, file_count * sizeof(char*));
            }
            size_t len = strlen(entry->d_name) + 1;
            files[file_count] = alloc(len);
            memcpy(files[file_count], entry->d_name, len);
            file_count++;
        }
    }
    closedir(dir);
#endif
    *out_count = file_count;
    return files;
}

char** get_directory_in_directory(const char* path_, u64* out_count) {
    char* path = make_portable_slashes(path_);
    u64 directory_count = 0;
    u64 directories_capacity = 8;
    char** directories = alloc(directories_capacity * sizeof(char*));

#if defined(_WIN32) || defined(_WIN64)
    WIN32_FIND_DATA find_file_data = {0};
    char search_path[MAX_PATH];
    snprintf(search_path, MAX_PATH, "%s/*", path);
    HANDLE hFind = FindFirstFile(search_path, &find_file_data);

    if (hFind == INVALID_HANDLE_VALUE) {
        return NULL;
    }
    for (;;) {
        if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            char* dir_name = find_file_data.cFileName;
            if (strcmp(dir_name, ".") == 0 || strcmp(dir_name, "..") == 0) {
                if (!FindNextFile(hFind, &find_file_data)) break;
                continue;
            }
            if (directory_count >= directories_capacity) {
                directories_capacity *= 2;
                void* old_directories = directories;
                directories = alloc(directories_capacity * sizeof(char*));
                memcpy(directories, old_directories, directory_count * sizeof(char*));
            }
            size_t len = strlen(find_file_data.cFileName) + 1;
            directories[directory_count] = alloc(len);
            memcpy(directories[directory_count], find_file_data.cFileName, len);
            directory_count++;
        }
        if (!FindNextFile(hFind, &find_file_data)) break;
    }
    FindClose(hFind);
#else
    DIR* dir = opendir(path);
    if (!dir) {
        return NULL;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char* dir_name = entry->d_name;
            if (strcmp(dir_name, ".") == 0 || strcmp(dir_name, "..") == 0) {
                continue;
            }
            if (directory_count >= directories_capacity) {
                directories_capacity *= 2;
                void* old_directories = directories;
                directories = alloc(directories_capacity * sizeof(char*));
                memcpy(directories, old_directories, directory_count * sizeof(char*));
            }
            size_t len = strlen(dir_name) + 1;
            directories[directory_count] = alloc(len);
            memcpy(directories[directory_count], dir_name, len);
            directory_count++;
        }
    }
    closedir(dir);
#endif
    *out_count = directory_count;
    return directories;
}
