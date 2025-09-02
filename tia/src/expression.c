#include "tia/expression.h"
#include <llvm-c/Core.h>
#include "tia.h"
#include "tia/ast.h"
#include "tia/type.h"

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
    expr.cast.expression = alloc(sizeof(Expression));
    *expr.cast.expression = *expression;
    return expr;
}

bool expression_can_implicitly_cast_without_deref(Type* expression, Type* type) {
    if (type_is_equal(expression, type)) return true;

    Type_Type expression_type_type = type_get_type(expression);
    Type_Type type_type = type_get_type(type);

    if (expression_type_type == type_number_literal && type_type == type_int) {
        return true;
    }
    if (expression_type_type == type_number_literal && type_type == type_float) {
        return true;
    }
    if (expression_type_type == type_int && type_type == type_uint) {
        return true;
    }

    if (expression_type_type == type_int && type_type == type_int) {
        u64 bits_expression = expression->base->int_.bits;
        u64 bits_type = type->base->int_.bits;
        if (bits_expression > bits_type) {
            return true;
        } else {
            return false;
        }
    }

    if (expression_type_type == type_float && type_type == type_float) {
        return true;
    }

    if (expression_type_type == type_uint && type_type == type_uint) {
        u64 bits_expression = expression->base->uint.bits;
        u64 bits_type = type->base->uint.bits;
        if (bits_expression > bits_type) {
            return true;
        } else {
            return false;
        }
    }

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

void expression_compile(Expression* expression, Function* func, Scope* scope, Type_Substitution_List* substitutions, Variable_LLVM_Value_List* var_to_llvm_val, LLVMValueRef function_value) {
    switch (expression->expr_type) {
        case et_number_literal:
            return expression_compile_number_literal(expression, func, scope, substitutions, var_to_llvm_val, function_value);
        case et_variable:
            return expression_compile_variable(expression, func, scope, substitutions, var_to_llvm_val, function_value);
        case et_biop:
            massert(false, "not implemented");
        case et_multi_expression:
            massert(false, "not implemented");
        case et_cast:
            return expression_compile_cast(expression, func, scope, substitutions, var_to_llvm_val, function_value);
        case et_invalid:
            massert(false, "unexpected expression");
    }
}

void expression_compile_number_literal(Expression* expression, Function* func, Scope* scope, Type_Substitution_List* substitutions, Variable_LLVM_Value_List* var_to_llvm_val, LLVMValueRef function_value) {
    // do nothing
}

void expression_compile_variable(Expression* expression, Function* func, Scope* scope, Type_Substitution_List* substitutions, Variable_LLVM_Value_List* var_to_llvm_val, LLVMValueRef function_value) {
    // do nothing
}

void expression_compile_cast(Expression* expression, Function* func, Scope* scope, Type_Substitution_List* substitutions, Variable_LLVM_Value_List* var_to_llvm_val, LLVMValueRef function_value) {
    Expression* from_expression = expression->cast.expression;
    expression_compile(from_expression, func, scope, substitutions, var_to_llvm_val, function_value);
    LLVMValueRef from_value = from_expression->value;

    Type to_type = type_get_real_type(&expression->type, substitutions);
    Type_Type to_type_type = type_get_type(&to_type);
    LLVMTypeRef to_type_llvm = type_get_llvm_type(&to_type, substitutions);
    Type from_type = type_get_real_type(&expression->cast.expression->type, substitutions);
    Type_Type from_type_type = type_get_type(&from_type);
    LLVMTypeRef from_type_llvm = type_get_llvm_type(&from_type, substitutions);

    if (type_is_reference_of(&from_type, &to_type)) {
        LLVMTypeRef to_type_llvm = type_get_llvm_type(&to_type, substitutions);
        expression->value = LLVMBuildLoad2(context.llvm_info.builder, to_type_llvm, from_value, "deref");
        return;
    }

    if (from_type_type == type_number_literal && (to_type_type == type_int || to_type_type == type_uint)) {
        Expression_Number_Literal from_number = expression_get_number_literal(from_expression);
        if (from_number.is_float) {
            i64 as_u64 = from_number.number_float;
            expression->value = LLVMConstInt(to_type_llvm, as_u64, false);
        } else {
            expression->value = LLVMConstIntOfString(to_type_llvm, from_number.number, 10);
        }
        return;
    }
    if (from_type_type == type_number_literal && to_type_type == type_float) {
        Expression_Number_Literal from_number = expression_get_number_literal(from_expression);
        if (from_number.is_float) {
            expression->value = LLVMConstReal(to_type_llvm, from_number.number_float);
        } else {
            expression->value = LLVMConstRealOfString(to_type_llvm, from_number.number);
        }
        return;
    }

    if ((from_type_type == type_int || from_type_type == type_uint) && to_type_type == type_int) {
        u64 to_bits = to_type.base->int_.bits;
        u64 from_bits = from_type.base->int_.bits;
        if (to_bits > from_bits) {
            expression->value = LLVMBuildSExt(context.llvm_info.builder, from_value, to_type_llvm, "sext");
        } else if (to_bits < from_bits) {
            expression->value = LLVMBuildTrunc(context.llvm_info.builder, from_value, to_type_llvm, "trunc");
        } else {
            expression->value = from_value;
        }
        return;
    }

    if ((from_type_type == type_uint || from_type_type == type_int) && to_type_type == type_uint) {
        u64 to_bits = to_type.base->uint.bits;
        u64 from_bits = from_type.base->uint.bits;
        if (to_bits > from_bits) {
            expression->value = LLVMBuildZExt(context.llvm_info.builder, from_value, to_type_llvm, "zext");
        } else if (to_bits < from_bits) {
            expression->value = LLVMBuildTrunc(context.llvm_info.builder, from_value, to_type_llvm, "trunc");
        } else {
            expression->value = from_value;
        }
        return;
    }

    if (from_type_type == type_float && to_type_type == type_float) {
        u64 to_bits = to_type.base->float_.bits;
        u64 from_bits = from_type.base->float_.bits;
        if (to_bits > from_bits) {
            expression->value = LLVMBuildFPExt(context.llvm_info.builder, from_value, to_type_llvm, "fpext");
        } else if (to_bits < from_bits) {
            expression->value = LLVMBuildFPTrunc(context.llvm_info.builder, from_value, to_type_llvm, "fptrunc");
        } else {
            expression->value = from_value;
        }
        return;
    }

    if (from_type_type == type_int && to_type_type == type_float) {
        expression->value = LLVMBuildSIToFP(context.llvm_info.builder, from_value, to_type_llvm, "int_to_float");
        return;
    }
    if (from_type_type == type_uint && to_type_type == type_float) {
        expression->value = LLVMBuildUIToFP(context.llvm_info.builder, from_value, to_type_llvm, "uint_to_float");
        return;
    }
    massert(false, "not implemented");
}

Expression_Number_Literal expression_get_number_literal_number_literal(Expression* expression) {
    massert(expression->expr_type == et_number_literal, "expression is not et_number_literal");
    Expression_Number_Literal number = {0};
    number.is_float = expression->number.is_float;
    if (number.is_float) {
        number.number_float = expression->number.number_float;
    } else {
        number.number = expression->number.number;
    }
    return number;
}

Expression_Number_Literal expression_get_number_literal(Expression* expression) {
    switch (expression->expr_type) {
        case et_biop:
            massert(false, "not implemented");
        case et_number_literal:
            return expression_get_number_literal_number_literal(expression);
        case et_variable:
        case et_multi_expression:
        case et_cast:
        case et_invalid:
            massert(false, "unexpected expression");
    }
}
