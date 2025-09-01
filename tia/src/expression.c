#include "tia.h"
#include "tia/ast.h"

Expression expression_create(Ast* ast, Scope* scope) {
    switch (ast->type) {
        case ast_word:
            return expression_create_word(ast, scope);
        case ast_multi_expression:
            return expression_create_multi_expression(ast, scope);
        case ast_number:
            return expression_create_number_literal(ast, scope);
        case ast_biop:
            return expression_create_biop(ast, scope);
        case ast_variable_declaration:
        case ast_return:
        case ast_end_statement:
        case ast_assignment:
        case ast_scope:
        case ast_string:
        case ast_file:
        case ast_function_declaration:
        case ast_type:
        case ast_invalid:
            massert(false, "unexpected expression");
    }
}

Expression expression_create_variable(Ast* ast, Scope* scope) {
    Expression expression = {0};
    expression.expr_type = et_variable;
    expression.ast = ast;
    Variable* variable = scope_get_variable(scope, ast->word.word);
    expression.variable.variable = variable;
    expression.type = variable->type;
    return expression;
}

Expression expression_create_biop(Ast* ast, Scope* scope) {
    massert(false, "not implemented");
}

Expression expression_create_word(Ast* ast, Scope* scope) {
    return expression_create_variable(ast, scope);
}

Expression expression_create_multi_expression(Ast* ast, Scope* scope) {
    Expression expression = {0};
    expression.expr_type = et_multi_expression;
    expression.ast = ast;
    Expression_List expressions = expression_list_create(2);
    for (u64 i = 0; i < ast->multi_expression.expressions.count; i++) {
        Ast* expression_ast = ast_list_get(&ast->multi_expression.expressions, i);
        Expression expression = expression_create(expression_ast, scope);
        if (expression.expr_type == et_invalid) return expression;
        expression_list_add(&expressions, &expression);
    }
    Type_List types = type_list_create(expressions.count);
    for (u64 i = 0; i < expressions.count; i++) {
        Expression* expression = expression_list_get(&expressions, i);
        Type type = expression->type;
        type_list_add(&types, &type);
    }
    expression.type = type_get_multi_value_type(&types);
    expression.multi_expression.expressions = expressions;
    return expression;
}

Expression expression_create_number_literal(Ast* ast, Scope* scope) {
    Expression expression = {0};
    expression.expr_type = et_number_literal;
    expression.type = type_get_number_literal_type();
    expression.ast = ast;
    massert(ast->type == ast_number, "ast is not ast_number");
    Ast_Number* number = &ast->number;
    expression.number.is_float = number->is_float;
    if (number->is_float) {
        expression.number.number_float = number->number_float;
    } else {
        expression.number.number = number->number;
    }
    return expression;
}

Expression expression_cast(Expression* expression, Type* type) {
    Expression expr;
    expr.expr_type = et_cast;
    expr.ast = expression->ast;
    expr.type = *type;
    expr.cast.expression = expression;
    return expr;
}

bool expression_can_implicitly_cast_without_deref(Type* expression, Type* type) {
    if (type_is_equal(expression, type)) return true;
    return false;
}

bool expression_can_implicitly_cast(Type* expression, Type* type) {
    bool can = expression_can_implicitly_cast_without_deref(expression, type);
    if (can) return true;

    Type_Type expression_type_type = type_get_type(expression);
    if (expression_type_type == type_ref) {
        Type deref_type = type_deref(expression);
        return expression_can_implicitly_cast(&deref_type, type);
    }
    return false;
}

Expression expression_implicitly_cast(Expression* expression, Type* type) {
    bool can = expression_can_implicitly_cast_without_deref(&expression->type, type);
    if (can) {
        Expression cast = expression_cast(expression, type);
        return cast;
    }

    Type_Type expression_type_type = type_get_type(&expression->type);
    if (expression_type_type == type_ref) {
        Type deref_type = type_deref(&expression->type);
        can = expression_can_implicitly_cast(&deref_type, type);
        if (can) {
            Expression cast = expression_cast(expression, type);
            return cast;
        }
    }

    char* expression_type_name = type_get_name(&expression->type);
    char* type_name = type_get_name(type);
    log_error_ast(expression->ast, "Can't cast %s to %s", expression_type_name, type_name);
    Expression err = {0};
    return err;
}
