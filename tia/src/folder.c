#include "tia.h"
#include "tia/expression.h"
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

void folder_call_init_functions(Folder* folder, LLVMBasicBlockRef block_to_call_from) {
    for (u64 i = 0; i < folder->dependencies.count; i++) {
        Folder* dependency = folder_pointer_list_get_folder(&folder->dependencies, i);
        folder_call_init_functions(dependency, block_to_call_from);
    }

    Function* init_function = folder->init_function;
    Function_Instance* init_function_instance = &init_function->instances.data[0];

    function_llvm_prototype_instance(init_function_instance);
    function_llvm_implement_instance(init_function_instance);

    LLVMPositionBuilderAtEnd(context.llvm_info.builder, block_to_call_from);

    LLVMValueRef init_function_value = init_function_instance->function_value;

    LLVMTypeRef init_function_type = type_get_llvm_type(&init_function_instance->type);

    LLVMBuildCall2(context.llvm_info.builder, init_function_type, init_function_value, NULL, 0, "");
}

Folder* folder_new(char* path) {
    Folder* folder = alloc(sizeof(Folder));
    memset(folder, 0, sizeof(Folder));
    folder_pointer_list_add(&context.folders, folder);

    folder->name = get_file_name(path);
    folder->path = path;
    folder->dependencies = folder_pointer_list_create(8);
    folder->called_init_function = false;
    u64 folder_path_count = strlen(path);

    const char* language_dir = getenv("Tia_Language_Dir");
    if (language_dir == NULL) {
        log_error("Tia_Language_Dir environment variable not set");
        return NULL;
    }
    u64 language_dir_len = strlen(language_dir);
    char* std_lib_path = alloc(language_dir_len + 8);
    snprintf(std_lib_path, language_dir_len + 8, "%sstd_lib", language_dir);
    if (strcmp(path, std_lib_path) != 0) {
        folder_add_dependency(folder, std_lib_path);
    }

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

    Function* init_function = function_new_init(folder);
    folder->init_function = init_function;

    for (u64 i = 0; i < folder->files.count; i++) {
        File* file = file_pointer_list_get_file(&folder->files, i);
        file_prototype_types(file);
    }

    for (u64 i = 0; i < folder->files.count; i++) {
        File* file = file_pointer_list_get_file(&folder->files, i);
        file_prototype_functions(file);
    }

    Scope* global_scope = &context.global_scope;
    Variable* exisiting_global_heap = scope_get_variable(global_scope, "global_heap");
    if (exisiting_global_heap == NULL) {
        Variable global_heap = {0};
        global_heap.name = alloc(strlen("global_heap") + 1);
        strcpy(global_heap.name, "global_heap");
        Type_Base* base = type_find_base_type("heap");
        Type type_find_type = {0};
        type_find_type.base = base;
        type_find_type.modifiers = type_modifier_list_create(0);
        type_find_type.modifiers.count = 0;
        type_find_type.ast = NULL;
        global_heap.type = type_find_type;
        Variable* var_in_global_scope = scope_add_variable(global_scope, &global_heap);

        Expression* allocator = alloc(sizeof(Expression));
        *allocator = expression_variable_access(var_in_global_scope);
        global_scope->default_allocator = allocator;
    }

    for (u64 i = 0; i < folder->files.count; i++) {
        File* file = file_pointer_list_get_file(&folder->files, i);
        file_implement_types(file);
    }

    for (u64 i = 0; i < folder->files.count; i++) {
        File* file = file_pointer_list_get_file(&folder->files, i);
        file_add_global_declarations(file, folder->init_function);
    }

    for (u64 i = 0; i < folder->files.count; i++) {
        File* file = file_pointer_list_get_file(&folder->files, i);
        file_check_for_invalid_global_statements(file);
    }

    for (u64 i = 0; i < folder->files.count; i++) {
        File* file = file_pointer_list_get_file(&folder->files, i);
        file_implement_functions(file);
    }

    return folder;
}
