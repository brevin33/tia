#pragma once
#include "tia/lists.h"

typedef struct Function Function;
typedef struct Folder {
    char* name;
    char* path;
    Folder_Pointer_List dependencies;
    File_Pointer_List files;
    Function* init_function;
    bool called_init_function;
} Folder;

Folder* folder_new(char* path);

void folder_add_dependency(Folder* folder, char* dependency_path);

void make_sure_main_does_not_use_interfaces(Folder* folder);

void folder_call_init_functions(Folder* folder, LLVMBasicBlockRef block_to_call_from);
