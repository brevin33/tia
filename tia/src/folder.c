#include "tia.h"
#include "tia/file.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996)  // disables 'unsafe' function warning
#endif

void make_sure_main_does_not_use_interfaces(Folder* folder) {
    for (u64 i = 0; i < context.functions.count; i++) {
        Function* function = context.functions.data[i];
        if (strcmp(function->name, "main") == 0) {
        }
    }
}

void folder_add_dependency(Folder* folder, char* dependency_path) {
    Folder* already_compiled = NULL;
    for (u64 i = 0; i < context.folders.count; i++) {
        Folder* existing = folder_pointer_list_get_folder(&context.folders, i);
        if (strcmp(existing->path, dependency_path) == 0) {
            already_compiled = existing;
            break;
        }
    }

    if (already_compiled != NULL) {
        folder_pointer_list_add(&folder->dependencies, already_compiled);
        return;
    }
    Folder* dependency = folder_new(dependency_path);
    folder_pointer_list_add(&folder->dependencies, dependency);
}

Folder* folder_new(char* path) {
    Folder* folder = alloc(sizeof(Folder));
    memset(folder, 0, sizeof(Folder));
    folder_pointer_list_add(&context.folders, folder);

    folder->name = get_file_name(path);
    folder->path = path;
    folder->dependencies = folder_pointer_list_create(8);
    u64 folder_path_count = strlen(path);

    const char* language_dir = getenv("Tia_Language_Dir");
    if (language_dir == NULL) {
        log_error("Tia_Language_Dir environment variable not set");
        return NULL;
    }
    u64 language_dir_len = strlen(language_dir);
    char* std_lib_path = alloc(language_dir_len + 8);
    snprintf(std_lib_path, language_dir_len + 8, "%sstd_lib", language_dir);

    folder_add_dependency(folder, std_lib_path);

    u64 file_count = 0;
    char** file_names = get_file_in_directory(path, &file_count);
    folder->files = file_pointer_list_create(file_count);
    for (u64 i = 0; i < file_count; i++) {
        char* file_extension = get_file_extension(file_names[i]);
        if (strcmp(file_extension, "tia") != 0) {
            continue;
        }
        u64 file_name_count = strlen(file_names[i]);
        u64 file_path_count = file_name_count + folder_path_count;
        char* file_path = alloc(file_path_count + 1);
        sprintf(file_path, "%s/%s", path, file_names[i]);
        File* file = file_new(file_path);
        file_pointer_list_add(&folder->files, file);
    }

    for (u64 i = 0; i < folder->files.count; i++) {
        File* file = file_pointer_list_get_file(&folder->files, i);
        file_prototype_types(file);
    }

    for (u64 i = 0; i < folder->files.count; i++) {
        File* file = file_pointer_list_get_file(&folder->files, i);
        file_prototype_functions(file);
    }

    for (u64 i = 0; i < folder->files.count; i++) {
        File* file = file_pointer_list_get_file(&folder->files, i);
        file_implement(file);
    }

    return folder;
}
