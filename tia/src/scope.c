
#include <llvm-c/Core.h>
#include "tia.h"

Scope scope_create(Scope* parent) {
    Scope scope = {0};
    scope.parent = parent;
    scope.statements = statement_list_create(8);
    scope.variables = variable_list_create(8);
    return scope;
}

Variable* scope_add_variable(Scope* scope, Variable* variable) {
    Variable* existing = NULL;
    for (u64 i = 0; i < scope->variables.count; i++) {
        Variable* existing_variable = variable_list_get(&scope->variables, i);
        if (strcmp(existing_variable->name, variable->name) == 0) {
            existing = existing_variable;
            break;
        }
    }
    if (existing != NULL) return NULL;

    Type_Type var_type_type = type_get_type(&variable->type);
    Variable var = *variable;
    var.is_ref = false;
    if (var_type_type == type_ref) {
        var.is_ref = true;
        Type deref_type = type_deref(&variable->type);
        var.type = deref_type;
    }

    Variable* in_list_variable = variable_list_add(&scope->variables, &var);
    return in_list_variable;
}

Variable* scope_get_variable(Scope* scope, char* name) {
    for (u64 i = 0; i < scope->variables.count; i++) {
        Variable* variable = variable_list_get(&scope->variables, i);
        if (strcmp(variable->name, name) == 0) {
            return variable;
        }
    }
    if (scope->parent != NULL) {
        return scope_get_variable(scope->parent, name);
    }
    return NULL;
}

bool scope_compile_scope(Scope* scope, Function* func, Type_Substitution_List* substitutions, Variable_LLVM_Value_List* var_to_llvm_val, LLVMValueRef function_value) {
    LLVMBasicBlockRef last_block = context.llvm_info.current_block;

    LLVMBasicBlockRef alloca_block = LLVMAppendBasicBlock(function_value, "alloca");
    LLVMBuildBr(context.llvm_info.builder, alloca_block);
    LLVMPositionBuilderAtEnd(context.llvm_info.builder, alloca_block);
    context.llvm_info.current_block = alloca_block;

    for (u64 i = 0; i < scope->variables.count; i++) {
        Variable* variable = variable_list_get(&scope->variables, i);
        Variable_LLVM_Value v = {0};
        v.variable = variable;
        if (variable->is_ref) {
            v.value = NULL;
        } else {
            LLVMTypeRef variable_type = type_get_llvm_type(&variable->type, substitutions);
            v.value = LLVMBuildAlloca(context.llvm_info.builder, variable_type, variable->name);
            variable_llvm_value_list_add(var_to_llvm_val, &v);
        }
    }
    bool exitedScope = false;
    for (u64 i = 0; i < scope->statements.count; i++) {
        Statement* statement = statement_list_get(&scope->statements, i);
        exitedScope = statement_compile(statement, func, scope, substitutions, var_to_llvm_val, function_value);
        if (exitedScope) {
            break;
        }
    }

    return exitedScope;
}

LLVMValueRef scope_get_variable_value(Variable_LLVM_Value_List* var_to_llvm_val, Variable* variable) {
    for (u64 i = 0; i < var_to_llvm_val->count; i++) {
        Variable_LLVM_Value* v = variable_llvm_value_list_get(var_to_llvm_val, i);
        if (v->variable == variable) {
            return v->value;
        }
    }
    massert(false, "variable not found");
    return NULL;
}

void scope_add_statements(Scope* scope, Ast* ast, Function* function) {
    massert(ast->type == ast_scope, "ast is not ast_type");
    Ast_Scope* scope_ast = &ast->scope;
    for (u64 i = 0; i < scope_ast->statements.count; i++) {
        Ast* statement_ast = ast_list_get(&scope_ast->statements, i);
        Statement statement = statement_create(statement_ast, scope, function);
        if (statement.type == st_invalid) continue;
        statement_list_add(&scope->statements, &statement);
    }
}
