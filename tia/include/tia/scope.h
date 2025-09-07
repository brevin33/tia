#pragma once
#include "tia/lists.h"
#include "tia/type.h"

typedef struct Function Function;

typedef struct Variable {
    char* name;
    Type type;
} Variable;

typedef struct Variable_LLVM_Value {
    LLVMValueRef value;
    Variable* variable;
} Variable_LLVM_Value;

typedef struct Scope Scope;
typedef struct Scope {
    Scope* parent;
    Statement_List statements;
    Variable_List variables;
    union {
        LLVMBasicBlockRef continue_block;
    };
} Scope;

Scope scope_create(Scope* parent);

Variable* scope_add_variable(Scope* scope, Variable* variable);

Variable* scope_get_variable(Scope* scope, char* name);

void scope_add_statements(Scope* scope, Ast* ast, Function* function);

LLVMValueRef scope_get_variable_value(Variable_LLVM_Value_List* var_to_llvm_val, Variable* variable);

bool scope_compile_scope(Scope* scope, Function* func, Type_Substitution_List* substitutions, Variable_LLVM_Value_List* var_to_llvm_val, LLVMValueRef function_value);
