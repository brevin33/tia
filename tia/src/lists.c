#include "tia.h"

Pointer_List pointer_list_create(u64 initial_size) {
    Pointer_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(void*));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

void** pointer_list_add(Pointer_List* list, void* pointer) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(void*));
        memcpy(list->data, old_data, list->count * sizeof(void*));
    }
    list->data[list->count] = pointer;
    list->count++;
    return &list->data[list->count - 1];
}

void** pointer_list_get(Pointer_List* list, u64 index) {
    return &list->data[index];
}

void* pointer_list_get_pointer(Pointer_List* list, u64 index) {
    return list->data[index];
}

void pointer_list_pop(Pointer_List* list) {
    list->count--;
}

Folder_Pointer_List folder_pointer_list_create(u64 initial_size) {
    Folder_Pointer_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(Folder*));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

Folder** folder_pointer_list_add(Folder_Pointer_List* list, Folder* folder) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(Folder*));
        memcpy(list->data, old_data, list->count * sizeof(Folder*));
    }
    list->data[list->count] = folder;
    list->count++;
    return &list->data[list->count - 1];
}

Folder** folder_pointer_list_get(Folder_Pointer_List* list, u64 index) {
    return &list->data[index];
}

Folder* folder_pointer_list_get_folder(Folder_Pointer_List* list, u64 index) {
    Folder** folder = folder_pointer_list_get(list, index);
    return *folder;
}

void folder_pointer_list_pop(Folder_Pointer_List* list) {
    list->count--;
}

File_Pointer_List file_pointer_list_create(u64 initial_size) {
    File_Pointer_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(File*));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

File** file_pointer_list_add(File_Pointer_List* list, File* file) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(File*));
        memcpy(list->data, old_data, list->count * sizeof(File*));
    }
    list->data[list->count] = file;
    list->count++;
    return &list->data[list->count - 1];
}

File** file_pointer_list_get(File_Pointer_List* list, u64 index) {
    return &list->data[index];
}

File* file_pointer_list_get_file(File_Pointer_List* list, u64 index) {
    File** id = file_pointer_list_get(list, index);
    return *id;
}

void file_pointer_list_pop(File_Pointer_List* list) {
    list->count--;
}

Token_List token_list_create(u64 initial_size) {
    Token_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(Token));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

Token* token_list_add(Token_List* list, Token* token) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(Token));
        memcpy(list->data, old_data, list->count * sizeof(Token));
    }
    list->data[list->count] = *token;
    list->count++;
    return &list->data[list->count - 1];
}

Token* token_list_get(Token_List* list, u64 index) {
    return &list->data[index];
}

void token_list_pop(Token_List* list) {
    list->count--;
}

u64_List u64_list_create(u64 initial_size) {
    u64_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(u64));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

u64* u64_list_add(u64_List* list, u64 value) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(u64));
        memcpy(list->data, old_data, list->count * sizeof(u64));
    }
    list->data[list->count] = value;
    list->count++;
    return &list->data[list->count - 1];
}

u64* u64_list_get(u64_List* list, u64 index) {
    return &list->data[index];
}

u64 u64_list_get_u64(u64_List* list, u64 index) {
    u64* value = u64_list_get(list, index);
    return *value;
}

void u64_list_pop(u64_List* list) {
    list->count--;
}

String_List string_list_create(u64 initial_size) {
    String_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(char*));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

char** string_list_add(String_List* list, char* value) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(char*));
        memcpy(list->data, old_data, list->count * sizeof(char*));
    }
    list->data[list->count] = value;
    list->count++;
    return &list->data[list->count - 1];
}

char** string_list_get(String_List* list, u64 index) {
    return &list->data[index];
}

char* string_list_get_string(String_List* list, u64 index) {
    char** value = string_list_get(list, index);
    return *value;
}

void string_list_pop(String_List* list) {
    list->count--;
}

Ast_List ast_list_create(u64 initial_size) {
    Ast_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(Ast));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

Ast* ast_list_add(Ast_List* list, Ast* ast) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(Ast));
        memcpy(list->data, old_data, list->count * sizeof(Ast));
    }
    list->data[list->count] = *ast;
    list->count++;
    return &list->data[list->count - 1];
}

Ast* ast_list_get(Ast_List* list, u64 index) {
    return &list->data[index];
}

void ast_list_pop(Ast_List* list) {
    list->count--;
}

Type_Base_Pointer_List type_base_pointer_list_create(u64 initial_size) {
    Type_Base_Pointer_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(Type_Base*));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

Type_Base** type_base_pointer_list_add(Type_Base_Pointer_List* list, Type_Base* type_base) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(Type_Base*));
        memcpy(list->data, old_data, list->count * sizeof(Type_Base*));
    }
    list->data[list->count] = type_base;
    list->count++;
    return &list->data[list->count - 1];
}

Type_Base** type_base_pointer_list_get(Type_Base_Pointer_List* list, u64 index) {
    return &list->data[index];
}

Type_Base* type_base_pointer_list_get_type_base(Type_Base_Pointer_List* list, u64 index) {
    Type_Base** type_base = type_base_pointer_list_get(list, index);
    return *type_base;
}

void type_base_pointer_list_pop(Type_Base_Pointer_List* list) {
    list->count--;
}

Statement_List statement_list_create(u64 initial_size) {
    Statement_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(Statement));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

Statement* statement_list_add(Statement_List* list, Statement* statement) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(Statement));
        memcpy(list->data, old_data, list->count * sizeof(Statement));
    }
    list->data[list->count] = *statement;
    list->count++;
    return &list->data[list->count - 1];
}

Statement* statement_list_get(Statement_List* list, u64 index) {
    return &list->data[index];
}

void statement_list_pop(Statement_List* list) {
    list->count--;
}

Variable_List variable_list_create(u64 initial_size) {
    Variable_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(Variable));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

Variable* variable_list_add(Variable_List* list, Variable* variable) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(Variable));
        memcpy(list->data, old_data, list->count * sizeof(Variable));
    }
    list->data[list->count] = *variable;
    list->count++;
    return &list->data[list->count - 1];
}

Variable* variable_list_get(Variable_List* list, u64 index) {
    return &list->data[index];
}

void variable_list_pop(Variable_List* list) {
    list->count--;
}

Type_List type_list_create(u64 initial_size) {
    Type_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(Type));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

Type* type_list_add(Type_List* list, Type* type) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(Type));
        memcpy(list->data, old_data, list->count * sizeof(Type));
    }
    list->data[list->count] = *type;
    list->count++;
    return &list->data[list->count - 1];
}

Type* type_list_get(Type_List* list, u64 index) {
    return &list->data[index];
}

void type_list_pop(Type_List* list) {
    list->count--;
}

Type_Modifier_List type_modifier_list_create(u64 initial_size) {
    Type_Modifier_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(Type_Modifier));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

Type_Modifier* type_modifier_list_add(Type_Modifier_List* list, Type_Modifier* type_modifier) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(Type_Modifier));
        memcpy(list->data, old_data, list->count * sizeof(Type_Modifier));
    }
    list->data[list->count] = *type_modifier;
    list->count++;
    return &list->data[list->count - 1];
}

Type_Modifier* type_modifier_list_get(Type_Modifier_List* list, u64 index) {
    return &list->data[index];
}

void type_modifier_list_pop(Type_Modifier_List* list) {
    list->count--;
}

Ast_Assignee_List ast_assignee_list_create(u64 initial_size) {
    Ast_Assignee_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(Ast_Assignee));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

Ast_Assignee* ast_assignee_list_add(Ast_Assignee_List* list, Ast_Assignee* ast_assignee) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(Ast_Assignee));
        memcpy(list->data, old_data, list->count * sizeof(Ast_Assignee));
    }
    list->data[list->count] = *ast_assignee;
    list->count++;
    return &list->data[list->count - 1];
}

Ast_Assignee* ast_assignee_list_get(Ast_Assignee_List* list, u64 index) {
    return &list->data[index];
}

void ast_assignee_list_pop(Ast_Assignee_List* list) {
    list->count--;
}

Expression_List expression_list_create(u64 initial_size) {
    Expression_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(Expression));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

Expression* expression_list_add(Expression_List* list, Expression* expression) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(Expression));
        memcpy(list->data, old_data, list->count * sizeof(Expression));
    }
    list->data[list->count] = *expression;
    list->count++;
    return &list->data[list->count - 1];
}

Expression* expression_list_get(Expression_List* list, u64 index) {
    return &list->data[index];
}

void expression_list_pop(Expression_List* list) {
    list->count--;
}

Assignees_List assignees_list_create(u64 initial_size) {
    Assignees_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(Assignee));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

Assignee* assignees_list_add(Assignees_List* list, Assignee* assignees) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(Assignee));
        memcpy(list->data, old_data, list->count * sizeof(Assignee));
    }
    list->data[list->count] = *assignees;
    list->count++;
    return &list->data[list->count - 1];
}

Assignee* assignees_list_get(Assignees_List* list, u64 index) {
    return &list->data[index];
}

void assignees_list_pop(Assignees_List* list) {
    list->count--;
}

Function_List function_list_create(u64 initial_size) {
    Function_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(Function));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

Function* function_list_add(Function_List* list, Function* function) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(Function));
        memcpy(list->data, old_data, list->count * sizeof(Function));
    }
    list->data[list->count] = *function;
    list->count++;
    return &list->data[list->count - 1];
}

Function* function_list_get(Function_List* list, u64 index) {
    return &list->data[index];
}

void function_list_pop(Function_List* list) {
    list->count--;
}

Function_Pointer_List function_pointer_list_create(u64 initial_size) {
    Function_Pointer_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(Function*));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

Function** function_pointer_list_add(Function_Pointer_List* list, Function* function) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(Function*));
        memcpy(list->data, old_data, list->count * sizeof(Function*));
    }
    list->data[list->count] = function;
    list->count++;
    return &list->data[list->count - 1];
}

Function** function_pointer_list_get(Function_Pointer_List* list, u64 index) {
    return &list->data[index];
}

Function* function_pointer_list_get_function(Function_Pointer_List* list, u64 index) {
    Function** function = function_pointer_list_get(list, index);
    return *function;
}

void function_pointer_list_pop(Function_Pointer_List* list) {
    list->count--;
}

Type_Substitution_List type_substitution_list_create(u64 initial_size) {
    Type_Substitution_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(Type_Substitution));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

Type_Substitution* type_substitution_list_add(Type_Substitution_List* list, Type_Substitution* type_substitution) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(Type_Substitution));
        memcpy(list->data, old_data, list->count * sizeof(Type_Substitution));
    }
    list->data[list->count] = *type_substitution;
    list->count++;
    return &list->data[list->count - 1];
}

Type_Substitution* type_substitution_list_get(Type_Substitution_List* list, u64 index) {
    return &list->data[index];
}

void type_substitution_list_pop(Type_Substitution_List* list) {
    list->count--;
}

Type_Substitution_List type_substitution_list_copy(Type_Substitution_List* list) {
    Type_Substitution_List substitutions = type_substitution_list_create(list->count);
    for (u64 i = 0; i < list->count; i++) {
        Type_Substitution* substitution = type_substitution_list_get(list, i);
        type_substitution_list_add(&substitutions, substitution);
    }
    return substitutions;
}

LLVMValueRef_List llvm_value_ref_list_create(u64 initial_size) {
    LLVMValueRef_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(LLVMValueRef));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

LLVMValueRef* llvm_value_ref_list_add(LLVMValueRef_List* list, LLVMValueRef value) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(LLVMValueRef));
        memcpy(list->data, old_data, list->count * sizeof(LLVMValueRef));
    }
    list->data[list->count] = value;
    list->count++;
    return &list->data[list->count - 1];
}

LLVMValueRef* llvm_value_ref_list_get(LLVMValueRef_List* list, u64 index) {
    return &list->data[index];
}

void llvm_value_ref_list_pop(LLVMValueRef_List* list) {
    list->count--;
}

LLVM_Type_Substitutions_To_LLVM_Value_List llvm_type_substitutions_to_llvm_value_list_create(u64 initial_size) {
    LLVM_Type_Substitutions_To_LLVM_Value_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(LLVM_Type_Substitutions_To_LLVM_Value));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

LLVM_Type_Substitutions_To_LLVM_Value* llvm_type_substitutions_to_llvm_value_list_add(LLVM_Type_Substitutions_To_LLVM_Value_List* list, LLVM_Type_Substitutions_To_LLVM_Value* llvm_type_substitutions_to_llvm_value) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(LLVM_Type_Substitutions_To_LLVM_Value));
        memcpy(list->data, old_data, list->count * sizeof(LLVM_Type_Substitutions_To_LLVM_Value));
    }
    list->data[list->count] = *llvm_type_substitutions_to_llvm_value;
    list->count++;
    return &list->data[list->count - 1];
}

LLVM_Type_Substitutions_To_LLVM_Value* llvm_type_substitutions_to_llvm_value_list_get(LLVM_Type_Substitutions_To_LLVM_Value_List* list, u64 index) {
    return &list->data[index];
}

void llvm_type_substitutions_to_llvm_value_list_pop(LLVM_Type_Substitutions_To_LLVM_Value_List* list) {
    list->count--;
}

Variable_LLVM_Value_List variable_llvm_value_list_create(u64 initial_size) {
    Variable_LLVM_Value_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(Variable_LLVM_Value));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

Variable_LLVM_Value* variable_llvm_value_list_add(Variable_LLVM_Value_List* list, Variable_LLVM_Value* variable_llvm_value) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(Variable_LLVM_Value));
        memcpy(list->data, old_data, list->count * sizeof(Variable_LLVM_Value));
    }
    list->data[list->count] = *variable_llvm_value;
    list->count++;
    return &list->data[list->count - 1];
}

Variable_LLVM_Value* variable_llvm_value_list_get(Variable_LLVM_Value_List* list, u64 index) {
    return &list->data[index];
}

void variable_llvm_value_list_pop(Variable_LLVM_Value_List* list) {
    list->count--;
}

Type_Interface_Function_List type_interface_function_list_create(u64 initial_size) {
    Type_Interface_Function_List list;
    if (initial_size == 0) {
        list.data = NULL;
        list.count = 0;
        list.capacity = 0;
    } else {
        list.data = alloc(initial_size * sizeof(Type_Interface_Function));
        list.count = 0;
        list.capacity = initial_size;
    }
    return list;
}

Type_Interface_Function* type_interface_function_list_add(Type_Interface_Function_List* list, Type_Interface_Function* type_interface_function) {
    if (list->count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 8;
        }
        list->capacity *= 2;
        void* old_data = list->data;
        list->data = alloc(list->capacity * sizeof(Type_Interface_Function));
        memcpy(list->data, old_data, list->count * sizeof(Type_Interface_Function));
    }
    list->data[list->count] = *type_interface_function;
    list->count++;
    return &list->data[list->count - 1];
}

Type_Interface_Function* type_interface_function_list_get(Type_Interface_Function_List* list, u64 index) {
    return &list->data[index];
}

void type_interface_function_list_pop(Type_Interface_Function_List* list) {
    list->count--;
}
