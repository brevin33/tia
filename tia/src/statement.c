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
        case ast_function_call:
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
            if (expression.expr_type == et_invalid) {
                Statement err = {0};
                return err;
            }
            Type* expression_type = &expression.type;
            Type_Type expression_type_type = type_get_type(expression_type);
            if (expression_type_type != type_ref) {
                log_error_ast(expression_ast, "Can't assign to non type");
                Statement err = {0};
                return err;
            }
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
                Assignee* assignee = assignees_list_get(&assignees, i);
                Type assignee_type = *statement_get_assignee_type(assignee);
                if (assignee->is_variable_declaration) {
                    if (assignee->variable->is_ref) {
                        Type ref_type = type_get_reference(&assignee_type);
                        assignee_type = ref_type;
                    } else {
                        // do nothing
                    }
                } else {
                    Type deref_type = type_deref(&assignee_type);
                    assignee_type = deref_type;
                }
                Expression value_expression = expression_implicitly_cast(&multi_expression->expressions.data[i], &assignee_type);
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
            Assignee* assignee = assignees_list_get(&assignees, 0);
            Type assignee_type = *statement_get_assignee_type(assignees_list_get(&assignees, 0));
            if (assignee->is_variable_declaration) {
                if (assignee->variable->is_ref) {
                    Type ref_type = type_get_reference(&assignee_type);
                    assignee_type = ref_type;
                } else {
                    // do nothing
                }
            } else {
                Type deref_type = type_deref(&assignee_type);
                assignee_type = deref_type;
            }
            Expression value_expression = expression_implicitly_cast(&value, &assignee_type);
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

bool statement_compile(Statement* statement, Function* func, Scope* scope, Type_Substitution_List* substitutions, Variable_LLVM_Value_List* var_to_llvm_val, LLVMValueRef function_value) {
    switch (statement->type) {
        case st_return:
            return statement_compile_return(statement, func, scope, substitutions, var_to_llvm_val, function_value);
        case st_ignore:
            return false;
        case st_assignment:
            return statement_compile_assignment(statement, func, scope, substitutions, var_to_llvm_val, function_value);
        case st_expression:
            return statement_compile_expression(statement, func, scope, substitutions, var_to_llvm_val, function_value);
        case st_scope:
            return statement_compile_scope(statement, func, scope, substitutions, var_to_llvm_val, function_value);
        case st_invalid:
            massert(false, "invalid statement");
            return false;
    }
}

bool statement_compile_return(Statement* statement, Function* func, Scope* scope, Type_Substitution_List* substitutions, Variable_LLVM_Value_List* var_to_llvm_val, LLVMValueRef function_value) {
    LLVMValueRef return_value = expression_compile(&statement->return_.return_value, func, scope, substitutions, var_to_llvm_val, function_value);
    LLVMBuildRet(context.llvm_info.builder, return_value);
    return true;
}

bool statement_compile_assignment(Statement* statement, Function* func, Scope* scope, Type_Substitution_List* substitutions, Variable_LLVM_Value_List* var_to_llvm_val, LLVMValueRef function_value) {
    massert(statement->assignment.assignees.count < 512, "too many assignees");
    LLVMValueRef assignee_values[512];
    for (u64 i = 0; i < statement->assignment.assignees.count; i++) {
        Assignee* assignee = assignees_list_get(&statement->assignment.assignees, i);
        if (assignee->is_variable_declaration) {
            // do nothing
        } else {
            Expression* expression = assignee->expression;
            assignee_values[i] = expression_compile(expression, func, scope, substitutions, var_to_llvm_val, function_value);
        }
    }
    LLVMValueRef llvmvalue = expression_compile(statement->assignment.value, func, scope, substitutions, var_to_llvm_val, function_value);

    if (statement->assignment.assignees.count == 1) {
        LLVMValueRef value = llvmvalue;
        Assignee* assignee = assignees_list_get(&statement->assignment.assignees, 0);
        if (assignee->is_variable_declaration) {
            Variable* variable = assignee->variable;
            if (variable->is_ref) {
                Variable_LLVM_Value v = {0};
                v.variable = variable;
                v.value = value;
                variable_llvm_value_list_add(var_to_llvm_val, &v);
            } else {
                LLVMValueRef variable_value = scope_get_variable_value(var_to_llvm_val, variable);
                LLVMBuildStore(context.llvm_info.builder, value, variable_value);
            }
        } else {
            Expression* expression_assignee = assignee->expression;
            LLVMValueRef assignee_value = assignee_values[0];
            LLVMBuildStore(context.llvm_info.builder, value, assignee_value);
        }
    } else {
        Expression* value_expression = statement->assignment.value;
        massert(value_expression->expr_type == et_multi_expression, "value_expression is not et_multi_expression");
        Expression_Multi_Expression* multi_expression = &value_expression->multi_expression;
        massert(multi_expression->expressions.count == statement->assignment.assignees.count, "multi_expression->expressions.count != statement->assignment.assignees.count");
        // multi value is special and actually return a pointer to LLVMValueRef array so we need to cast it.
        // might be better but this is the only case where returning a single llvm value ref doesn't work
        LLVMValueRef* multi_value = (LLVMValueRef*)llvmvalue;
        for (u64 i = 0; i < multi_expression->expressions.count; i++) {
            Assignee* assignee = assignees_list_get(&statement->assignment.assignees, i);
            Expression* expression = expression_list_get(&multi_expression->expressions, i);

            if (assignee->is_variable_declaration) {
                Variable* variable = assignee->variable;
                if (variable->is_ref) {
                    Variable_LLVM_Value v = {0};
                    v.variable = variable;
                    v.value = multi_value[i];
                    variable_llvm_value_list_add(var_to_llvm_val, &v);
                } else {
                    LLVMValueRef variable_value = scope_get_variable_value(var_to_llvm_val, variable);
                    LLVMBuildStore(context.llvm_info.builder, multi_value[i], variable_value);
                }
            } else {
                Expression* expression_assignee = assignee->expression;
                LLVMValueRef assignee_value = assignee_values[i];
                LLVMValueRef expression_value = multi_value[i];
                LLVMBuildStore(context.llvm_info.builder, expression_value, assignee_value);
            }
        }
    }

    return false;
}

bool statement_compile_expression(Statement* statement, Function* func, Scope* scope, Type_Substitution_List* substitutions, Variable_LLVM_Value_List* var_to_llvm_val, LLVMValueRef function_value) {
    expression_compile(&statement->expression.expression, func, scope, substitutions, var_to_llvm_val, function_value);
    return false;
}

bool statement_compile_scope(Statement* statement, Function* func, Scope* scope, Type_Substitution_List* substitutions, Variable_LLVM_Value_List* var_to_llvm_val, LLVMValueRef function_value) {
    massert(false, "not implemented");
    return false;
}
