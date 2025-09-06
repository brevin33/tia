#include "tia.h"

char* file_get_line(File* file, u64 line_number) {
    massert(line_number < file->line_start_indexes.count, "line number is out of bounds");
    char* file_contents = file->contents;
    u64_List line_start_indexes = file->line_start_indexes;
    u64 line_start_index = u64_list_get_u64(&line_start_indexes, line_number);

    u64 line_end_index = line_start_index;
    while (file_contents[line_end_index] != '\n' && file_contents[line_end_index] != '\0') line_end_index++;

    u64 line_count = line_end_index - line_start_index;
    char* line = alloc(line_count + 1);
    memcpy(line, &file_contents[line_start_index], line_count);
    line[line_count] = '\0';

    return line;
}

void file_prototype_types(File* file) {
    Ast* ast = file->ast;
    Ast_File* file_ast = &ast->file;
    for (u64 i = 0; i < file_ast->global_declarations.count; i++) {
        Ast* global_declaration = ast_list_get(&file_ast->global_declarations, i);
        switch (global_declaration->type) {
            case ast_interface: {
                Type_Base* type_base = type_prototype_interface(global_declaration);
                type_base_pointer_list_add(&file->types, type_base);
                break;
            }
            default:
                break;
        }
    }
}

void file_prototype_functions(File* file) {
    Ast* ast = file->ast;
    Ast_File* file_ast = &ast->file;
    for (u64 i = 0; i < file_ast->global_declarations.count; i++) {
        Ast* global_declaration = ast_list_get(&file_ast->global_declarations, i);
        if (global_declaration->type == ast_function_declaration) {
            Ast_Function_Declaration* function_declaration = &global_declaration->function_declaration;
            Function* function = function_new(global_declaration);
            function_pointer_list_add(&file->functions, function);
        }
    }
}

void file_implement(File* file) {
    for (u64 i = 0; i < file->types.count; i++) {
        Type_Base* type_base = type_base_pointer_list_get_type_base(&file->types, i);
        type_implement_interface(type_base);
    }

    for (u64 i = 0; i < file->functions.count; i++) {
        Function* function = function_pointer_list_get_function(&file->functions, i);
        function_implement(function);
    }
}

File* file_new(char* path) {
    File* file = alloc(sizeof(File));
    memset(file, 0, sizeof(File));
    file_pointer_list_add(&context.files, file);

    file->path = path;
    file->name = get_file_name(path);
    file->contents = read_file(path);
    if (file->contents == NULL) {
        red_printf("Could not read file %s\n", path);
        abort();
    }
    file->line_start_indexes = file_get_line_start_indexes(file);
    file->tokens = token_get_tokens(file);
    file->ast = alloc(sizeof(Ast));
    *file->ast = ast_create(file->tokens.data);
    file->functions = function_pointer_list_create(8);

    return file;
}

u64_List file_get_line_start_indexes(File* file) {
    u64_List line_start_indexes = u64_list_create(8);
    u64_list_add(&line_start_indexes, 0);
    for (u64 i = 0; file->contents[i] != '\0'; i++) {
        if (file->contents[i] == '\n') {
            u64_list_add(&line_start_indexes, i + 1);
        }
    }
    return line_start_indexes;
}

bool has_main_function(File* file) {
    for (u64 i = 0; i < file->functions.count; i++) {
        Function* function = function_pointer_list_get_function(&file->functions, i);
        if (strcmp(function->name, "main") == 0) {
            return true;
        }
    }
    return false;
}

u64 file_get_lines(File* file, u64 t_start_index, u64 t_end_index, u64* out_count) {
    char* file_contents = file->contents;

    u64 start_index;
    for (start_index = t_start_index; start_index > 0; start_index--)
        if (file_contents[start_index] == '\n') break;
    if (start_index > 0) start_index++;  // skip the newline

    u64 end_index;
    for (end_index = t_end_index; file_contents[end_index] != '\0'; end_index++)
        if (file_contents[end_index] == '\n') break;
    if (file_contents[end_index] == '\n') end_index--;  // skip the newline

    u64 line_starts_count = 1;
    for (u64 i = start_index; i < end_index; i++) {
        if (file_contents[i] == '\n') {
            line_starts_count++;
        }
    }

    u64_List line_start_indexes = file->line_start_indexes;
    for (u64 i = 0; i < line_start_indexes.count; i++) {
        u64 line_start_index = u64_list_get_u64(&line_start_indexes, i);
        if (start_index == line_start_index) {
            u64 line = i;
            *out_count = line_starts_count;
            return line;
        }
    }
    massert(false, "Could not find line start index");
    return 0;
}
