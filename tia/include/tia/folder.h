#pragma once
#include "tia/lists.h"

typedef struct Folder {
    char* name;
    char* path;
    Folder_Pointer_List dependencies;
    File_Pointer_List files;
} Folder;

Folder* folder_new(char* path);

void make_sure_main_does_not_use_interfaces(Folder* folder);
