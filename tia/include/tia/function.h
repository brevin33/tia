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
    Type_Substitution_List constant_substitutions;
    union {
        LLVM_Function_Info llvm_info;
    };
    u64 interface_instance_number_count_down_from;
} Function;

Function* function_new(Ast* ast);

void function_implement(Function* function);

Type* function_get_return_type(Function* function);

Type* function_get_parameter_type(Function* function, u64 index);

char* function_get_mangled_name(Function* function, Type_Substitution_List* substitutions);

// returns the interface instance number for this
u64 function_add_constant_substitution(Function* function, Type_Substitution* substitution);

void function_add_constant_substitution_with_interface_instance_number(Function* function, Type_Substitution* substitution, u64 interface_instance_number);

Type_Substitution function_find_constant_substitution(Function* function, u64 interface_instance_number);

typedef struct Function_Find_Result {
    Function* function;
    Type_Substitution_List substitutions;
    Type there_is_an_interface_function_return_type;
} Function_Find_Result;

Function_Find_Result function_find(Type_List* parameters, char* name, Ast* ast, bool log_error, bool allow_interface_function);

LLVMValueRef function_compile_llvm(Function* function, Type_Substitution_List* substitutions);

LLVMValueRef function_get_llvm_value(Function* function, Type_Substitution_List* substitutions);
