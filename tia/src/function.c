#include "tia.h"
#include "tia/ast.h"

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

Type* function_get_parameter_type(Function* function, u64 index) {
    Type function_type = function->type;
    massert(function_type.base->type == type_function, "function_type is not type_function");
    Type_Function* function_type_base = &function_type.base->function;
    Type* parameter_type = type_list_get(&function_type_base->parameters, index);
    return parameter_type;
}
