#pragma once
#include "tia/lists.h"
#include "tia/type.h"

typedef struct Function Function;

typedef struct Variable {
    char* name;
    Type type;
    LLVMValueRef value;
} Variable;

typedef struct Scope Scope;
typedef struct Scope {
    Scope* parent;
    Statement_List statements;
    Variable_List variables;
    Template_To_Type template_to_type;
    union {
        LLVMBasicBlockRef continue_block;
    };
} Scope;

Scope scope_create(Scope* parent);

Variable* scope_add_variable(Scope* scope, Variable* variable);

Variable* scope_get_variable(Scope* scope, char* name);

void scope_add_statements(Scope* scope, Ast* ast, Function_Instance* function);

bool scope_compile_scope(Scope* scope, Function_Instance* func);

Type* scope_get_templated_type(Scope* scope, char* name);

Template_Map* scope_add_template(Scope* scope, char* name, Type* type);
