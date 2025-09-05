#pragma once
#include <llvm-c/Types.h>
#include "tia/basic.h"
#include "tia/lists.h"
#include "tia/scope.h"

typedef struct LLVM_Type_Substitutions_To_LLVM_Value {
    Type_Substitution_List substitutions;
    LLVMValueRef value;
} LLVM_Type_Substitutions_To_LLVM_Value;

typedef struct LLVM_Function_Info {
    LLVM_Type_Substitutions_To_LLVM_Value_List function_value_by_substitutions;
} LLVM_Function_Info;

typedef struct Function {
    Type type;
    char* name;
    Scope parameters_scope;
    Scope body_scope;
    Ast* ast;
    union {
        LLVM_Function_Info llvm_info;
    };
} Function;

Function* function_new(Ast* ast);

void function_implement(Function* function);

Type* function_get_return_type(Function* function);

Type* function_get_parameter_type(Function* function, u64 index);

char* function_get_mangled_name(Function* function, Type_Substitution_List* substitutions);

typedef struct Function_Find_Result {
    Function* function;
    Type_Substitution_List substitutions;
} Function_Find_Result;

Function_Find_Result function_find(Type_List* parameters, char* name, Ast* ast);

LLVMValueRef function_compile_llvm(Function* function, Type_Substitution_List* substitutions);

LLVMValueRef function_get_llvm_value(Function* function, Type_Substitution_List* substitutions);
