#include "tia.h"
#include "tia/file.h"

Folder* folder_new(char* path) {
    Folder* folder = alloc(sizeof(Folder));
    memset(folder, 0, sizeof(Folder));
    folder_pointer_list_add(&context.folders, folder);

    folder->name = get_file_name(path);
    folder->path = path;
    folder->dependencies = folder_pointer_list_create(8);
    u64 folder_path_count = strlen(path);

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
        file_prototype_functions(file);
    }

    for (u64 i = 0; i < folder->files.count; i++) {
        File* file = file_pointer_list_get_file(&folder->files, i);
        file_implement(file);
    }

    return folder;
}
