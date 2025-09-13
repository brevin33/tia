#pragma once
#include "tia/basic.h"
#include "tia/llvm.h"

typedef struct {
    void** data;
    u64 count;
    u64 capacity;
} Pointer_List;

Pointer_List pointer_list_create(u64 initial_size);
void** pointer_list_add(Pointer_List* list, void* pointer);
void** pointer_list_get(Pointer_List* list, u64 index);
void* pointer_list_get_pointer(Pointer_List* list, u64 index);
void pointer_list_pop(Pointer_List* list);

typedef struct Folder Folder;
typedef struct {
    Folder** data;
    u64 count;
    u64 capacity;
} Folder_Pointer_List;

Folder_Pointer_List folder_pointer_list_create(u64 initial_size);
Folder** folder_pointer_list_add(Folder_Pointer_List* list, Folder* folder);
Folder** folder_pointer_list_get(Folder_Pointer_List* list, u64 index);
Folder* folder_pointer_list_get_folder(Folder_Pointer_List* list, u64 index);
void folder_pointer_list_pop(Folder_Pointer_List* list);

typedef struct File File;
typedef struct {
    File** data;
    u64 count;
    u64 capacity;
} File_Pointer_List;

File_Pointer_List file_pointer_list_create(u64 initial_size);
File** file_pointer_list_add(File_Pointer_List* list, File* file);
File** file_pointer_list_get(File_Pointer_List* list, u64 index);
File* file_pointer_list_get_file(File_Pointer_List* list, u64 index);
void file_pointer_list_pop(File_Pointer_List* list);

typedef struct Token Token;
typedef struct {
    Token* data;
    u64 count;
    u64 capacity;
} Token_List;

Token_List token_list_create(u64 initial_size);
Token* token_list_add(Token_List* list, Token* token);
Token* token_list_get(Token_List* list, u64 index);
void token_list_pop(Token_List* list);

typedef struct {
    u64* data;
    u64 count;
    u64 capacity;
} u64_List;

u64_List u64_list_create(u64 initial_size);
u64* u64_list_add(u64_List* list, u64 value);
u64* u64_list_get(u64_List* list, u64 index);
u64 u64_list_get_u64(u64_List* list, u64 index);
void u64_list_pop(u64_List* list);

typedef struct {
    char** data;
    u64 count;
    u64 capacity;
} String_List;

String_List string_list_create(u64 initial_size);
char** string_list_add(String_List* list, char* value);
char** string_list_get(String_List* list, u64 index);
char* string_list_get_string(String_List* list, u64 index);
void string_list_pop(String_List* list);

typedef struct Ast Ast;
typedef struct {
    Ast* data;
    u64 count;
    u64 capacity;
} Ast_List;

Ast_List ast_list_create(u64 initial_size);
Ast* ast_list_add(Ast_List* list, Ast* ast);
Ast* ast_list_get(Ast_List* list, u64 index);
void ast_list_pop(Ast_List* list);

typedef struct Type_Base Type_Base;
typedef struct {
    Type_Base** data;
    u64 count;
    u64 capacity;
} Type_Base_Pointer_List;

Type_Base_Pointer_List type_base_pointer_list_create(u64 initial_size);
Type_Base** type_base_pointer_list_add(Type_Base_Pointer_List* list, Type_Base* type_base);
Type_Base** type_base_pointer_list_get(Type_Base_Pointer_List* list, u64 index);
Type_Base* type_base_pointer_list_get_type_base(Type_Base_Pointer_List* list, u64 index);
void type_base_pointer_list_pop(Type_Base_Pointer_List* list);

typedef struct Statement Statement;
typedef struct {
    Statement* data;
    u64 count;
    u64 capacity;
} Statement_List;

Statement_List statement_list_create(u64 initial_size);
Statement* statement_list_add(Statement_List* list, Statement* statement);
Statement* statement_list_get(Statement_List* list, u64 index);
void statement_list_pop(Statement_List* list);

typedef struct Variable Variable;
typedef struct {
    Variable* data;
    u64 count;
    u64 capacity;
} Variable_List;

Variable_List variable_list_create(u64 initial_size);
Variable* variable_list_add(Variable_List* list, Variable* variable);
Variable* variable_list_get(Variable_List* list, u64 index);
void variable_list_pop(Variable_List* list);

typedef struct Type Type;
typedef struct {
    Type* data;
    u64 count;
    u64 capacity;
} Type_List;

Type_List type_list_create(u64 initial_size);
Type* type_list_add(Type_List* list, Type* type);
Type* type_list_get(Type_List* list, u64 index);
void type_list_pop(Type_List* list);

typedef struct Type_Modifier Type_Modifier;
typedef struct {
    Type_Modifier* data;
    u64 count;
    u64 capacity;
} Type_Modifier_List;

Type_Modifier_List type_modifier_list_create(u64 initial_size);
Type_Modifier* type_modifier_list_add(Type_Modifier_List* list, Type_Modifier* type_modifier);
Type_Modifier* type_modifier_list_get(Type_Modifier_List* list, u64 index);
void type_modifier_list_pop(Type_Modifier_List* list);

typedef struct Ast_Assignee Ast_Assignee;
typedef struct {
    Ast_Assignee* data;
    u64 count;
    u64 capacity;
} Ast_Assignee_List;

Ast_Assignee_List ast_assignee_list_create(u64 initial_size);
Ast_Assignee* ast_assignee_list_add(Ast_Assignee_List* list, Ast_Assignee* ast_assignee);
Ast_Assignee* ast_assignee_list_get(Ast_Assignee_List* list, u64 index);
void ast_assignee_list_pop(Ast_Assignee_List* list);

typedef struct Expression Expression;
typedef struct {
    Expression* data;
    u64 count;
    u64 capacity;
} Expression_List;

Expression_List expression_list_create(u64 initial_size);
Expression* expression_list_add(Expression_List* list, Expression* expression);
Expression* expression_list_get(Expression_List* list, u64 index);
void expression_list_pop(Expression_List* list);

typedef struct Assignee Assignee;
typedef struct {
    Assignee* data;
    u64 count;
    u64 capacity;
} Assignees_List;

Assignees_List assignees_list_create(u64 initial_size);
Assignee* assignees_list_add(Assignees_List* list, Assignee* assignees);
Assignee* assignees_list_get(Assignees_List* list, u64 index);
void assignees_list_pop(Assignees_List* list);

typedef struct Function Function;
typedef struct {
    Function* data;
    u64 count;
    u64 capacity;
} Function_List;

Function_List function_list_create(u64 initial_size);
Function* function_list_add(Function_List* list, Function* function);
Function* function_list_get(Function_List* list, u64 index);
void function_list_pop(Function_List* list);

typedef struct {
    Function** data;
    u64 count;
    u64 capacity;
} Function_Pointer_List;

Function_Pointer_List function_pointer_list_create(u64 initial_size);
Function** function_pointer_list_add(Function_Pointer_List* list, Function* function);
Function** function_pointer_list_get(Function_Pointer_List* list, u64 index);
Function* function_pointer_list_get_function(Function_Pointer_List* list, u64 index);
void function_pointer_list_pop(Function_Pointer_List* list);

typedef struct {
    LLVMValueRef* data;
    u64 count;
    u64 capacity;
} LLVMValueRef_List;

LLVMValueRef_List llvm_value_ref_list_create(u64 initial_size);
LLVMValueRef* llvm_value_ref_list_add(LLVMValueRef_List* list, LLVMValueRef value);
LLVMValueRef* llvm_value_ref_list_get(LLVMValueRef_List* list, u64 index);
void llvm_value_ref_list_pop(LLVMValueRef_List* list);

typedef struct Function_Instance Function_Instance;
typedef struct Function_Instance_List {
    Function_Instance* data;
    u64 count;
    u64 capacity;
} Function_Instance_List;

Function_Instance_List function_instance_list_create(u64 initial_size);
Function_Instance* function_instance_list_add(Function_Instance_List* list, Function_Instance* function_instance);
Function_Instance* function_instance_list_get(Function_Instance_List* list, u64 index);
void function_instance_list_pop(Function_Instance_List* list);

typedef struct Template_Map Template_Map;
typedef struct {
    Template_Map* data;
    u64 count;
    u64 capacity;
} Template_Map_List;

Template_Map_List template_map_list_create(u64 initial_size);
Template_Map* template_map_list_add(Template_Map_List* list, Template_Map* template_map);
Template_Map* template_map_list_get(Template_Map_List* list, u64 index);
void template_map_list_pop(Template_Map_List* list);
