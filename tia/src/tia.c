#include "tia.h"

Context context = {0};

static void context_init(Arena* arena) {
    context.arena = arena;
    context.folders = folder_pointer_list_create(8);
    context.files = file_pointer_list_create(8);
    context.types = type_base_pointer_list_create(8);
    context.functions = function_pointer_list_create(8);
    init_types();
}

Folder* create_tia_project(char* path, Arena* arena) {
    context_init(arena);
    Folder* folder = folder_new(path);
    return folder;
}
