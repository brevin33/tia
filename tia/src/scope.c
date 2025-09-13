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

    Variable var = *variable;

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

bool scope_compile_scope(Scope* scope, Function_Instance* func) {
    LLVMValueRef function_value = func->function_value;
    LLVMBasicBlockRef alloca_block = LLVMAppendBasicBlock(function_value, "alloca");
    LLVMBuildBr(context.llvm_info.builder, alloca_block);
    LLVMPositionBuilderAtEnd(context.llvm_info.builder, alloca_block);

    for (u64 i = 0; i < scope->variables.count; i++) {
        Variable* variable = variable_list_get(&scope->variables, i);
        Type* variable_type = &variable->type;
        Type_Type real_variable_type_type = type_get_type(variable_type);
        if (real_variable_type_type == type_ref) {
            variable->value = NULL;
            // don't add here we add in the variable declaration
        } else {
            LLVMTypeRef variable_type = type_get_llvm_type(&variable->type);
            variable->value = LLVMBuildAlloca(context.llvm_info.builder, variable_type, variable->name);
        }
    }
    bool exitedScope = false;
    for (u64 i = 0; i < scope->statements.count; i++) {
        Statement* statement = statement_list_get(&scope->statements, i);
        exitedScope = statement_compile(statement, func, scope);
        if (exitedScope) {
            break;
        }
    }

    return exitedScope;
}

void scope_add_statements(Scope* scope, Ast* ast, Function_Instance* function) {
    massert(ast->type == ast_scope, "ast is not ast_type");
    Ast_Scope* scope_ast = &ast->scope;
    for (u64 i = 0; i < scope_ast->statements.count; i++) {
        Ast* statement_ast = ast_list_get(&scope_ast->statements, i);
        Statement statement = statement_create(statement_ast, scope, function);
        if (statement.type == st_invalid) continue;
        statement_list_add(&scope->statements, &statement);
    }
}
