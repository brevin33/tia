#include "tia/expression.h"
#include <llvm-c/Core.h>
#include "tia.h"
#include "tia/ast.h"
#include "tia/basic.h"
#include "tia/function.h"
#include "tia/lists.h"
#include "tia/type.h"

Expression expression_create(Ast* ast, Scope* scope, Function_Instance* in_function) {
    switch (ast->type) {
        case ast_word:
            return expression_create_word(ast, scope, in_function);
        case ast_multi_expression:
            return expression_create_multi_expression(ast, scope, in_function);
        case ast_number:
            return expression_create_number_literal(ast, scope, in_function);
        case ast_biop:
            return expression_create_biop(ast, scope, in_function);
        case ast_function_call:
            return expression_create_function_call(ast, scope, in_function);
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

Expression expression_create_function_call(Ast* ast, Scope* scope, Function_Instance* in_function) {
    massert(ast->type == ast_function_call, "ast is not ast_function_call");
    Expression expression = {0};
    expression.expr_type = et_function_call;
    expression.ast = ast;
    char* function_name = ast->function_call.function_name;
    Expression_List arguments = expression_list_create(ast->function_call.arguments.count);
    Type_List parameter_types = type_list_create(ast->function_call.arguments.count);
    for (u64 i = 0; i < ast->function_call.arguments.count; i++) {
        Ast* argument_ast = ast_list_get(&ast->function_call.arguments, i);
        Expression argument = expression_create(argument_ast, scope, in_function);
        if (argument.expr_type == et_invalid) return argument;
        expression_list_add(&arguments, &argument);
        Type* argument_type = &argument.type;
        type_list_add(&parameter_types, argument_type);
    }

    Function_Instance* function_instance = function_find(&parameter_types, function_name, ast, true);
    if (function_instance == NULL) {
        Expression err = {0};
        return err;
    }
    expression.function_call.function_instance = function_instance;
    for (u64 i = 0; i < ast->function_call.arguments.count; i++) {
        Expression* argument = expression_list_get(&arguments, i);
        Type* parameter_type = &function_instance->type.base->function.parameters.data[i];
        Expression argument_cast = expression_implicitly_cast(argument, parameter_type);
        massert(argument_cast.expr_type != et_invalid, "should never happen");
        *argument = argument_cast;
    }
    expression.function_call.arguments = arguments;
    expression.type = function_instance->type.base->function.return_type;
    return expression;
}

Expression expression_create_variable(Ast* ast, Scope* scope, Function_Instance* in_function) {
    Expression expression = {0};
    expression.expr_type = et_variable;
    expression.ast = ast;
    Variable* variable = scope_get_variable(scope, ast->word.word);
    if (variable == NULL) {
        log_error_ast(ast, "Variable %s not found", ast->word.word);
        Expression err = {0};
        return err;
    }
    expression.variable.variable = variable;
    Type_Type var_type_type = type_get_type(&variable->type);
    if (var_type_type != type_ref) {
        Type ref_type = type_get_reference(&variable->type);
        expression.type = ref_type;
    } else {
        expression.type = variable->type;
    }
    return expression;
}

Expression expression_create_biop(Ast* ast, Scope* scope, Function_Instance* in_function) {
    massert(false, "not implemented");
}

Expression expression_create_word(Ast* ast, Scope* scope, Function_Instance* in_function) {
    return expression_create_variable(ast, scope, in_function);
}

Expression expression_create_multi_expression(Ast* ast, Scope* scope, Function_Instance* in_function) {
    Expression expression = {0};
    expression.expr_type = et_multi_expression;
    expression.ast = ast;
    Expression_List expressions = expression_list_create(2);
    for (u64 i = 0; i < ast->multi_expression.expressions.count; i++) {
        Ast* expression_ast = ast_list_get(&ast->multi_expression.expressions, i);
        Expression expression = expression_create(expression_ast, scope, in_function);
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

Expression expression_create_number_literal(Ast* ast, Scope* scope, Function_Instance* in_function) {
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
        if (bits_expression <= bits_type) {
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
        if (bits_expression <= bits_type) {
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
            Expression* deref = alloc(sizeof(Expression));
            *deref = expression_cast(expression, &deref_type);
            Expression cast = expression_cast(deref, type);
            return cast;
        }
    }

    char* expression_type_name = type_get_name(&expression->type);
    char* type_name = type_get_name(type);
    log_error_ast(expression->ast, "Can't cast %s to %s", expression_type_name, type_name);
    Expression err = {0};
    return err;
}

LLVMValueRef expression_compile(Expression* expression, Function_Instance* func, Scope* scope) {
    switch (expression->expr_type) {
        case et_number_literal:
            return expression_compile_number_literal(expression, func, scope);
        case et_variable:
            return expression_compile_variable(expression, func, scope);
        case et_biop:
            massert(false, "not implemented");
            return NULL;
        case et_multi_expression:
            return expression_compile_multi_expression(expression, func, scope);
        case et_cast:
            return expression_compile_cast(expression, func, scope);
        case et_function_call:
            return expression_compile_function_call(expression, func, scope);
        case et_invalid:
            massert(false, "unexpected expression");
            return NULL;
    }
}

// multi value is special and actually return a pointer to LLVMValueRef array so we need to cast it.
LLVMValueRef expression_compile_multi_expression(Expression* expression, Function_Instance* func, Scope* scope) {
    massert(expression->expr_type == et_multi_expression, "expression is not et_multi_expression");
    Expression_Multi_Expression* multi_expression = &expression->multi_expression;
    LLVMValueRef* mulit_value = alloc(sizeof(LLVMValueRef) * multi_expression->expressions.count);
    massert(multi_expression->expressions.count < 512, "too many expressions");
    for (u64 i = 0; i < multi_expression->expressions.count; i++) {
        Expression* expression = expression_list_get(&multi_expression->expressions, i);
        mulit_value[i] = expression_compile(expression, func, scope);
    }
    // multi value is special and actually return a pointer to LLVMValueRef array so we need to cast it.
    return (LLVMValueRef)mulit_value;
}

LLVMValueRef expression_compile_function_call(Expression* expression, Function_Instance* func, Scope* scope) {
    massert(expression->expr_type == et_function_call, "expression is not et_function_call");
    Expression_Function_Call* function_call = &expression->function_call;
    LLVMValueRef_List argument_values = llvm_value_ref_list_create(function_call->arguments.count);
    for (u64 i = 0; i < function_call->arguments.count; i++) {
        Expression* argument = expression_list_get(&function_call->arguments, i);
        LLVMValueRef argument_value = expression_compile(argument, func, scope);
        llvm_value_ref_list_add(&argument_values, argument_value);
    }

    Function_Instance* to_call = function_call->function_instance;
    LLVMValueRef function_value = to_call->function_value;
    massert(function_value != NULL, "function_value is NULL");
    LLVMTypeRef function_type = type_get_llvm_type(&to_call->type);

    LLVMValueRef result = LLVMBuildCall2(context.llvm_info.builder, function_type, function_value, argument_values.data, argument_values.count, "call");
    return result;
}

LLVMValueRef expression_compile_number_literal(Expression* expression, Function_Instance* func, Scope* scope) {
    // do nothing
    return NULL;
}

LLVMValueRef expression_compile_variable(Expression* expression, Function_Instance* func, Scope* scope) {
    Variable* variable = expression->variable.variable;
    return variable->value;
}

LLVMValueRef expression_compile_cast(Expression* expression, Function_Instance* func, Scope* scope) {
    Expression* from_expression = expression->cast.expression;
    LLVMValueRef from_value = expression_compile(from_expression, func, scope);

    Type to_type = expression->type;
    Type_Type to_type_type = type_get_type(&to_type);
    LLVMTypeRef to_type_llvm = type_get_llvm_type(&to_type);
    Type from_type = expression->cast.expression->type;
    Type_Type from_type_type = type_get_type(&from_type);

    if (type_is_equal(&from_type, &to_type)) {
        return from_value;
    }

    if (type_is_reference_of(&from_type, &to_type)) {
        LLVMTypeRef to_type_llvm = type_get_llvm_type(&to_type);
        return LLVMBuildLoad2(context.llvm_info.builder, to_type_llvm, from_value, "deref");
    }

    if (from_type_type == type_number_literal && (to_type_type == type_int || to_type_type == type_uint)) {
        Expression_Number_Literal from_number = expression_get_number_literal(from_expression);
        if (from_number.is_float) {
            i64 as_u64 = from_number.number_float;
            return LLVMConstInt(to_type_llvm, as_u64, false);
        } else {
            return LLVMConstIntOfString(to_type_llvm, from_number.number, 10);
        }
    }
    if (from_type_type == type_number_literal && to_type_type == type_float) {
        Expression_Number_Literal from_number = expression_get_number_literal(from_expression);
        if (from_number.is_float) {
            return LLVMConstReal(to_type_llvm, from_number.number_float);
        } else {
            return LLVMConstRealOfString(to_type_llvm, from_number.number);
        }
    }

    if ((from_type_type == type_int || from_type_type == type_uint) && to_type_type == type_int) {
        u64 to_bits = to_type.base->int_.bits;
        u64 from_bits = from_type.base->int_.bits;
        if (to_bits > from_bits) {
            return LLVMBuildSExt(context.llvm_info.builder, from_value, to_type_llvm, "sext");
        } else if (to_bits < from_bits) {
            return LLVMBuildTrunc(context.llvm_info.builder, from_value, to_type_llvm, "trunc");
        } else {
            return from_value;
        }
    }

    if ((from_type_type == type_uint || from_type_type == type_int) && to_type_type == type_uint) {
        u64 to_bits = to_type.base->uint.bits;
        u64 from_bits = from_type.base->uint.bits;
        if (to_bits > from_bits) {
            return LLVMBuildZExt(context.llvm_info.builder, from_value, to_type_llvm, "zext");
        } else if (to_bits < from_bits) {
            return LLVMBuildTrunc(context.llvm_info.builder, from_value, to_type_llvm, "trunc");
        } else {
            return from_value;
        }
    }

    if (from_type_type == type_float && to_type_type == type_float) {
        u64 to_bits = to_type.base->float_.bits;
        u64 from_bits = from_type.base->float_.bits;
        if (to_bits > from_bits) {
            return LLVMBuildFPExt(context.llvm_info.builder, from_value, to_type_llvm, "fpext");
        } else if (to_bits < from_bits) {
            return LLVMBuildFPTrunc(context.llvm_info.builder, from_value, to_type_llvm, "fptrunc");
        } else {
            return from_value;
        }
    }

    if (from_type_type == type_int && to_type_type == type_float) {
        return LLVMBuildSIToFP(context.llvm_info.builder, from_value, to_type_llvm, "int_to_float");
    }
    if (from_type_type == type_uint && to_type_type == type_float) {
        return LLVMBuildUIToFP(context.llvm_info.builder, from_value, to_type_llvm, "uint_to_float");
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
        case et_function_call:
        case et_variable:
        case et_multi_expression:
        case et_cast:
        case et_invalid:
            massert(false, "unexpected expression");
    }
}
