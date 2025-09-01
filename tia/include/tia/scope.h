#pragma once
#include "tia/lists.h"
#include "tia/type.h"

typedef struct Function Function;

typedef struct Variable {
    char* name;
    Type type;
} Variable;

typedef struct Scope Scope;
typedef struct Scope {
    Scope* parent;
    Statement_List statements;
    Variable_List variables;
} Scope;

Scope scope_create(Scope* parent);

Variable* scope_add_variable(Scope* scope, Variable* variable);

Variable* scope_get_variable(Scope* scope, char* name);

void scope_add_statements(Scope* scope, Ast* ast, Function* function);
