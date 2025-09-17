#include "tia.h"

Scope scope_create(Scope* parent) {
    Scope scope = {0};
    scope.parent = parent;
    scope.statements = statement_list_create(8);
    scope.variables = variable_list_create(8);
    scope.alloc_expressions = expression_list_create(1);
    scope.default_allocator = NULL;
    return scope;
}

Variable* scope_add_variable(Scope* scope, Variable* variable) {
    if (variable->type.base->type == type_compile_time_type) {
        Variable* new = alloc(sizeof(Variable));
        *new = *variable;
        return new;
    }
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

Variable* scope_get_variable(Scope* scope, const char* name) {
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

Type* scope_get_templated_type(Scope* scope, char* name) {
    for (u64 i = 0; i < scope->template_to_type.templates.count; i++) {
        Template_Map* template_map = template_map_list_get(&scope->template_to_type.templates, i);
        if (strcmp(template_map->name, name) == 0) {
            return &template_map->type;
        }
    }
    if (scope->parent != NULL) {
        return scope_get_templated_type(scope->parent, name);
    }
    return NULL;
}

Expression* scope_get_default_allocator(Scope* scope) {
    if (scope->default_allocator != NULL) {
        return scope->default_allocator;
    }
    if (scope->parent == NULL) {
        return NULL;
    }
    return scope_get_default_allocator(scope->parent);
}

Template_Map* scope_add_template(Scope* scope, char* name, Type* type) {
    // make sure the template doesn't already exist
    for (u64 i = 0; i < scope->template_to_type.templates.count; i++) {
        Template_Map* template_map = template_map_list_get(&scope->template_to_type.templates, i);
        if (strcmp(template_map->name, name) == 0) {
            return NULL;
        }
    }
    Template_Map template_map = {0};
    template_map.name = name;
    template_map.type = *type;
    return template_map_list_add(&scope->template_to_type.templates, &template_map);
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
            if (variable->type.base->type == type_compile_time_type) {
                variable->value = NULL;
            } else {
                variable->value = LLVMBuildAlloca(context.llvm_info.builder, variable_type, variable->name);
            }
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

    for (u64 i = 0; i < scope->alloc_expressions.count; i++) {
        Expression* expression = expression_list_get(&scope->alloc_expressions, i);
        massert(expression->expr_type == et_alloc, "expression is not et_alloc");
        Expression* allocator = type_get_allocator(&expression->type);
        if (allocator == expression) {
            Expression* default_allocator = scope_get_default_allocator(scope);
            type_set_allocator(&expression->type, default_allocator);

            Expression_List args = expression_list_create(2);
            expression_list_add(&args, default_allocator);
            expression_list_add(&args, expression->alloc.type_argument);

            Function_Instance* function_instance = function_find(&args, "alloc", expression->ast, true);
            expression->alloc.function_instance = function_instance;
        }
        massert(expression->alloc.function_instance != NULL, "function_instance is NULL");
    }
}
