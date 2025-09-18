#pragma once
#include <llvm-c/Types.h>
#include "tia/basic.h"
#include "tia/lists.h"
#include "tia/scope.h"

typedef struct Function_Instance {
    Type type;
    Function* function;
    Scope parameters_scope;
    union {
        Scope body_scope;
        char* extern_c_real_name;
    };
    LLVMValueRef function_value;
    Template_To_Type template_to_type;
    Type_List parameter_types;
} Function_Instance;

typedef struct Function {
    char* name;
    Ast* ast;
    Variable_List parameters;
    Type return_type;
    Function_Instance_List instances;
    bool is_extern_c;
} Function;

Function* function_new(Ast* ast);

Function* function_new_init(Folder* folder);

void function_implement(Function_Instance* function);

void function_prototype_instance(Function_Instance* function_instance);

void function_setup_instance_from_function(Function_Instance* function_instance);

void function_llvm_prototype_instance(Function_Instance* function_instance);

void function_llvm_implement_instance(Function_Instance* function_instance);

Function_Instance* function_find(Expression_List* parameters_exprs, const char* name, Ast* ast, bool log_error);

Type* function_get_parameter_type(Function_Instance* function, u64 index);

char* function_get_mangled_name(Function_Instance* function);
