#pragma once
#include "tia/basic.h"
#include "tia/lists.h"
#include "tia/scope.h"

typedef struct Function {
    Type type;
    char* name;
    Scope parameters_scope;
    Scope body_scope;
    Ast* ast;
} Function;

Function* function_new(Ast* ast);

void function_implement(Function* function);

Type* function_get_return_type(Function* function);

Type* function_get_parameter_type(Function* function, u64 index);
