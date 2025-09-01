#pragma once
#include "tia/basic.h"
#include "tia/lists.h"

typedef struct Ast Ast;
typedef struct File {
    char* contents;
    u64_List line_start_indexes;
    char* name;
    char* path;
    Token_List tokens;
    Ast* ast;
    Function_Pointer_List functions;
} File;

File* file_new(char* path);

void file_prototype_functions(File* file);

void file_implement(File* file);

char* file_get_line(File* file, u64 line_number);

u64_List file_get_line_start_indexes(File* file);

u64 file_get_lines(File* file, u64 t_start_index, u64 t_end_index, u64* out_count);
