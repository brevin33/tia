#include <sys/stat.h>
#include "tia.h"
#include "tia/expression.h"

Statement statement_create(Ast* ast, Scope* scope, Function* function) {
    switch (ast->type) {
        case ast_type:
        case ast_word:
        case ast_string:
        case ast_number:
        case ast_biop:
        case ast_multi_expression:
            return statement_create_expression(ast, scope, function);
        case ast_scope:
            return statement_create_scope(ast, scope, function);
        case ast_return:
            return statement_create_return(ast, scope, function);
        case ast_end_statement:
            return statement_create_ignore(ast, scope, function);
        case ast_assignment:
            return statement_create_assignment(ast, scope, function);
        case ast_variable_declaration:
        case ast_file:
        case ast_function_declaration:
        case ast_invalid: {
            massert(false, "unexpected statement");
            Statement err = {0};
            log_error_ast(ast, "Unexpected statement");
            return err;
        }
    }
}

Statement statement_create_assignment(Ast* ast, Scope* scope, Function* function) {
    massert(ast->type == ast_assignment, "ast is not ast_assignment");
    Ast_Assignment* assignment_ast = &ast->assignment;
    Assignees_List assignees = assignees_list_create(2);
    for (u64 i = 0; i < assignment_ast->assignees.count; i++) {
        Ast_Assignee* assignee_ast = ast_assignee_list_get(&assignment_ast->assignees, i);
        Assignee assignee = {0};
        if (assignee_ast->is_variable_declaration) {
            assignee.is_variable_declaration = true;
            Ast_Variable_Declaration* variable_declaration = &assignee_ast->variable_declaration->variable_declaration;
            char* variable_name = variable_declaration->name;
            Variable variable = {0};
            variable.name = variable_name;
            variable.type = type_find_ast(variable_declaration->type, true);
            Variable* in_list_variable = scope_add_variable(scope, &variable);
            if (in_list_variable == NULL) {
                log_error_ast(assignee_ast->variable_declaration, "Already declared variable with name %s", variable_name);
                Statement err = {0};
                return err;
            }
            assignee.variable = in_list_variable;
        } else {
            assignee.is_variable_declaration = false;
            Ast* expression_ast = assignee_ast->expression;
            Expression expression = expression_create(expression_ast, scope);
            assignee.expression = alloc(sizeof(Expression));
            *assignee.expression = expression;
        }
        assignees_list_add(&assignees, &assignee);
    }

    Ast* value_ast = assignment_ast->value;
    if (value_ast != NULL) {
        Expression value = expression_create(value_ast, scope);
        if (value.expr_type == et_invalid) {
            Statement err = {0};
            return err;
        }

        if (value.expr_type == et_multi_expression) {
            Expression_Multi_Expression* multi_expression = &value.multi_expression;
            if (multi_expression->expressions.count != assignees.count) {
                u64 values_count = multi_expression->expressions.count;
                u64 assignees_count = assignees.count;
                log_error_ast(ast, "Can't assign %llu values to %llu variables", values_count, assignees_count);
                Statement err = {0};
                return err;
            }
            for (u64 i = 0; i < multi_expression->expressions.count; i++) {
                Type* assignee_type = statement_get_assignee_type(assignees_list_get(&assignees, i));
                Expression value_expression = expression_implicitly_cast(&multi_expression->expressions.data[i], assignee_type);
                if (value_expression.expr_type == et_invalid) {
                    Statement err = {0};
                    return err;
                }
                Expression* value_expression_ptr = &multi_expression->expressions.data[i];
                *value_expression_ptr = value_expression;
            }
        } else {
            if (assignees.count != 1) {
                // TODO: maybe lets structs breakdown into multiple values
                log_error_ast(ast, "Can't assign to multiple variables with a value");
                Statement err = {0};
                return err;
            }
            Type* assignee_type = statement_get_assignee_type(assignees_list_get(&assignees, 0));
            Expression value_expression = expression_implicitly_cast(&value, assignee_type);
            if (value_expression.expr_type == et_invalid) {
                Statement err = {0};
                return err;
            }
            value = value_expression;
        }
        Statement statement = {0};
        statement.type = st_assignment;
        statement.ast = ast;
        statement.assignment.assignees = assignees;
        statement.assignment.value = alloc(sizeof(Expression));
        *statement.assignment.value = value;
        return statement;
    } else {
        Statement statement = {0};
        statement.type = st_assignment;
        statement.ast = ast;
        statement.assignment.assignees = assignees;
        statement.assignment.value = NULL;
        return statement;
    }
}

Statement statement_create_return(Ast* ast, Scope* scope, Function* function) {
    massert(ast->type == ast_return, "ast is not ast_return");
    Ast_Return* return_ast = &ast->return_;
    Ast* return_value_ast = return_ast->return_value;
    Expression return_value = expression_create(return_value_ast, scope);
    if (return_value.expr_type == et_invalid) {
        Statement err = {0};
        return err;
    }

    Type* function_return_type = function_get_return_type(function);
    Expression return_value_cast = expression_implicitly_cast(&return_value, function_return_type);
    if (return_value_cast.expr_type == et_invalid) {
        Statement err = {0};
        return err;
    }
    return_value = return_value_cast;

    Statement statement = {0};
    statement.type = st_return;
    statement.ast = ast;
    statement.return_.return_value = return_value;
    return statement;
}

Statement statement_create_scope(Ast* ast, Scope* scope, Function* function) {
    massert(ast->type == ast_scope, "ast is not ast_scope");
    Ast_Scope* scope_ast = &ast->scope;
    Scope new_scope = scope_create(scope);
    for (u64 i = 0; i < scope_ast->statements.count; i++) {
        Ast* statement_ast = ast_list_get(&scope_ast->statements, i);
        Statement statement = statement_create(statement_ast, &new_scope, function);
        if (statement.type == st_invalid) continue;
        statement_list_add(&new_scope.statements, &statement);
    }
    Statement statement = {0};
    statement.type = st_scope;
    statement.ast = ast;
    statement.scope.scope = new_scope;
    return statement;
}

Statement statement_create_expression(Ast* ast, Scope* scope, Function* function) {
    Expression expression = expression_create(ast, scope);
    if (expression.expr_type == et_invalid) {
        Statement err = {0};
        return err;
    }
    Statement statement = {0};
    statement.type = st_expression;
    statement.ast = ast;
    statement.expression.expression = expression;
    return statement;
}

Statement statement_create_ignore(Ast* ast, Scope* scope, Function* function) {
    Statement statement = {0};
    statement.type = st_ignore;
    statement.ast = ast;
    return statement;
}

Type* statement_get_assignee_type(Assignee* assignee) {
    if (assignee->is_variable_declaration) {
        return &assignee->variable->type;
    } else {
        return &assignee->expression->type;
    }
}
