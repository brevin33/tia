#include "tia/function.h"
#include "tia.h"
#include "tia/ast.h"
#include "tia/type.h"

void function_implement(Function* function) {
    Ast* ast = function->ast;
    Ast* body_ast = ast->function_declaration.body;
    if (body_ast == NULL) return;

    scope_add_statements(&function->body_scope, body_ast, function);
}

Function* function_new(Ast* ast) {
    massert(ast->type == ast_function_declaration, "ast is not ast_function_declaration");
    Ast_Function_Declaration* function_declaration = &ast->function_declaration;
    Function* function = alloc(sizeof(Function));
    memset(function, 0, sizeof(Function));
    function_pointer_list_add(&context.functions, function);

    function->ast = ast;
    function->parameters_scope = scope_create(NULL);
    function->body_scope = scope_create(&function->parameters_scope);

    Ast* return_type_ast = function_declaration->return_type;
    Type return_type = type_find_ast(return_type_ast, true);

    u64 parameter_count = function_declaration->parameters.count;
    Type_List parameters = type_list_create(parameter_count);
    for (u64 i = 0; i < parameter_count; i++) {
        Ast* parameter_ast = ast_list_get(&function_declaration->parameters, i);
        massert(parameter_ast->type == ast_variable_declaration, "parameter_ast is not ast_variable_declaration");

        Ast_Variable_Declaration* parameter_declaration = &parameter_ast->variable_declaration;

        Ast* parameter_type_ast = parameter_declaration->type;
        Type parameter_type = type_find_ast(parameter_type_ast, true);
        type_list_add(&parameters, &parameter_type);

        Variable variable = {0};
        variable.name = parameter_declaration->name;
        variable.type = parameter_type;
        Variable* in_list_variable = scope_add_variable(&function->parameters_scope, &variable);
        if (in_list_variable == NULL) {
            log_error_ast(parameter_ast, "Multiple declarations of parameter with name %s", parameter_declaration->name);
        }
    }
    function->type = type_get_function_type(&parameters, return_type);
    function->name = function_declaration->name;

    return function;
}

Type* function_get_return_type(Function* function) {
    Type function_type = function->type;
    massert(function_type.base->type == type_function, "function_type is not type_function");
    Type_Function* function_type_base = &function_type.base->function;
    return &function_type_base->return_type;
}

char* function_get_mangled_name(Function* function, Type_Substitution_List* substitutions) {
    Type real_function_type = type_get_real_type(&function->type, substitutions);
    Type_List* parameters = &real_function_type.base->function.parameters;
    Type return_type = real_function_type.base->function.return_type;

    u64 len = 0;
    char* return_type_name = type_get_name(&return_type);
    u64 return_type_name_len = strlen(return_type_name);
    len += return_type_name_len;
    String_List parameter_names = string_list_create(8);
    for (u64 i = 0; i < parameters->count; i++) {
        Type* parameter = type_list_get(parameters, i);
        char* parameter_name = type_get_name(parameter);
        string_list_add(&parameter_names, parameter_name);
        u64 parameter_name_len = strlen(parameter_name);
        len += parameter_name_len;
    }
    // add extra for spaces
    char* function_name = function->name;
    u64 function_name_len = strlen(function_name);
    len += parameters->count + 7 + function_name_len;
    u64 index = 0;
    char* name = alloc(len + 1);
    memcpy(name, return_type_name, return_type_name_len);
    index += return_type_name_len;
    name[index] = '$';
    index++;
    memcpy(name + index, function_name, function_name_len);
    index += function_name_len;
    name[index] = '(';
    index++;
    for (u64 i = 0; i < parameters->count; i++) {
        char* parameter_name = string_list_get_string(&parameter_names, i);
        u64 parameter_name_len = strlen(parameter_name);
        memcpy(name + index, parameter_name, parameter_name_len);
        index += parameter_name_len;
        if (i < parameters->count - 1) {
            name[index] = ',';
            index++;
        }
    }
    name[index] = ')';
    index++;
    name[index] = '\0';
    return name;
}

Type* function_get_parameter_type(Function* function, u64 index) {
    Type function_type = function->type;
    massert(function_type.base->type == type_function, "function_type is not type_function");
    Type_Function* function_type_base = &function_type.base->function;
    Type* parameter_type = type_list_get(&function_type_base->parameters, index);
    return parameter_type;
}

LLVMValueRef function_get_llvm_value(Function* function, Type_Substitution_List* substitutions) {
    for (u64 i = 0; i < function->llvm_info.function_value_by_substitutions.count; i++) {
        LLVM_Type_Substitutions_To_LLVM_Value* v = llvm_type_substitutions_to_llvm_value_list_get(&function->llvm_info.function_value_by_substitutions, i);
        if (type_substitutions_is_equal(&v->substitutions, substitutions)) {
            LLVMValueRef function_value = v->value;
            return function_value;
        }
    }
    return function_compile_llvm(function, substitutions);
}

LLVMValueRef function_compile_llvm(Function* function, Type_Substitution_List* substitutions) {
    Type* function_type = &function->type;
    LLVMTypeRef function_type_llvm = type_get_llvm_type(function_type, substitutions);
    char* mangled_name = function_get_mangled_name(function, substitutions);

    LLVMValueRef function_value = LLVMAddFunction(context.llvm_info.module, mangled_name, function_type_llvm);
    LLVM_Type_Substitutions_To_LLVM_Value_List* function_value_by_substitutions = &function->llvm_info.function_value_by_substitutions;
    LLVM_Type_Substitutions_To_LLVM_Value v = {0};
    v.substitutions = type_substitution_list_copy(substitutions);
    v.value = function_value;
    llvm_type_substitutions_to_llvm_value_list_add(function_value_by_substitutions, &v);

    LLVMBasicBlockRef last_block = NULL;
    if (context.llvm_info.current_block != NULL) {
        last_block = context.llvm_info.current_block;
    }

    LLVMBasicBlockRef entry_block = LLVMAppendBasicBlock(function_value, "entry");
    LLVMPositionBuilderAtEnd(context.llvm_info.builder, entry_block);
    context.llvm_info.current_block = entry_block;

    Variable_LLVM_Value_List var_to_llvm_val = variable_llvm_value_list_create(8);
    for (u64 i = 0; i < function->parameters_scope.variables.count; i++) {
        Variable* variable = variable_list_get(&function->parameters_scope.variables, i);
        Variable_LLVM_Value v = {0};
        v.variable = variable;
        LLVMValueRef param_value = LLVMGetParam(function_value, i);
        if (variable->is_ref) {
            v.value = param_value;
        } else {
            LLVMTypeRef param_type = type_get_llvm_type(&variable->type, substitutions);
            v.value = LLVMBuildAlloca(context.llvm_info.builder, param_type, variable->name);
            variable_llvm_value_list_add(&var_to_llvm_val, &v);
        }
    }

    if (last_block != NULL) {
        context.llvm_info.current_block = last_block;
        LLVMPositionBuilderAtEnd(context.llvm_info.builder, last_block);
    }

    scope_compile_scope(&function->body_scope, function, substitutions, &var_to_llvm_val, function_value);

    return function_value;
}
