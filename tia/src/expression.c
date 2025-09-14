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
        case ast_member_access:
            return expression_create_member_access(ast, scope, in_function);
        case ast_type:
            return expression_create_type(ast, scope, in_function, true);
        case ast_alloc:
            return expression_create_alloc(ast, scope, in_function);
        case ast_free:
            return expression_create_free(ast, scope, in_function);
        case ast_variable_declaration:
        case ast_return:
        case ast_struct_declaration:
        case ast_end_statement:
        case ast_assignment:
        case ast_scope:
        case ast_string:
        case ast_file:
        case ast_function_declaration:
        case ast_invalid:
            massert(false, "unexpected expression");
    }
}

Expression expression_access_struct_field(Expression* expression, u64 member_index) {
    Type* expression_type = &expression->type;
    Type_Type expression_type_type = type_get_type(expression_type);
    if (expression_type->base->type != type_struct) {
        log_error_ast(expression->ast, "Can't access struct field of non struct");
        Expression err = {0};
        return err;
    }
    // TODO: handle pointers and other levels of type modifiers
    if (expression_type->modifiers.count != 0) {
        if (expression_type->modifiers.count != 1) {
            massert(false, "not implemented");
        }
        if (expression_type_type != type_ref) {
            massert(false, "not implemented");
        }
    }

    Expression expr;
    expr.expr_type = et_struct_access;
    expr.ast = expression->ast;
    expr.struct_access.value = alloc(sizeof(Expression));
    *expr.struct_access.value = *expression;
    expr.struct_access.member_index = member_index;

    Type_Struct_Field* field = type_struct_field_list_get(&expression_type->base->struct_.fields, member_index);
    Type* field_type = &field->type;

    if (expression_type_type == type_ref) {
        expr.type = type_get_reference(field_type);
    } else {
        expr.type = *field_type;
    }
    return expr;
}

Expression expression_create_get_val(Ast* ast, Scope* scope, Function_Instance* in_function, Expression* value) {
    Expression expression = {0};
    expression.expr_type = et_get_val;
    expression.ast = ast;
    expression.get_val.expression = alloc(sizeof(Expression));
    *expression.get_val.expression = *value;
    expression.type = type_deref(&value->type);
    return expression;
}

Expression expression_create_get_ptr(Ast* ast, Scope* scope, Function_Instance* in_function, Expression* value) {
    Expression expression = {0};
    expression.expr_type = et_get_ptr;
    expression.ast = ast;
    expression.get_ptr.expression = alloc(sizeof(Expression));
    *expression.get_ptr.expression = *value;
    Type underlying_type = type_underlying(&value->type);
    expression.type = type_get_ptr(&underlying_type);
    return expression;
}

Expression expression_create_struct_access(Ast* ast, Scope* scope, Function_Instance* in_function, Expression* value) {
    massert(ast->type == ast_member_access, "ast is not ast_member_access");
    Expression expression = {0};
    expression.expr_type = et_struct_access;
    expression.ast = ast;
    Ast_Member_Access* member_access = &ast->member_access;
    expression.struct_access.value = alloc(sizeof(Expression));
    *expression.struct_access.value = *value;

    Type* value_type = &expression.struct_access.value->type;
    Type_Type value_type_type = type_get_type(value_type);
    Type_Type value_type_type_base = value_type->base->type;
    if (value_type_type_base != type_struct) {
        log_error_ast(ast->member_access.value, "Can't access struct member of non struct");
        Expression err = {0};
        return err;
    }

    // TODO: handel pointers and other levels of type modifiers
    if (value_type->modifiers.count != 0) {
        if (value_type->modifiers.count != 1) {
            massert(false, "not implemented");
        }
        if (value_type_type != type_ref) {
            massert(false, "not implemented");
        }
    }

    char* member_name = member_access->member_name;
    bool error;
    u64 number = get_string_uint(member_name, &error);
    if (error) {
        for (u64 i = 0; i < value_type->base->struct_.fields.count; i++) {
            Type_Struct_Field* field = type_struct_field_list_get(&value_type->base->struct_.fields, i);
            if (strcmp(field->name, member_name) == 0) {
                number = i;
                break;
            }
        }
    } else {
        if (number >= value_type->base->struct_.fields.count) {
            log_error_ast(ast->member_access.value, "No struct element of %llu", number);
            Expression err = {0};
            return err;
        }
    }
    expression.struct_access.member_index = number;

    Type_Struct_Field* field = type_struct_field_list_get(&value_type->base->struct_.fields, number);
    Type* field_type = &field->type;

    Type accessed_type;
    if (value_type_type == type_ref) {
        accessed_type = type_get_reference(field_type);
    } else {
        accessed_type = *field_type;
    }
    expression.type = accessed_type;

    return expression;
}

Expression expression_create_get_type_size(Ast* ast, Scope* scope, Function_Instance* in_function, Expression* value) {
    Type_Type value_type_type = type_get_type(&value->type);
    if (value_type_type != type_compile_time_type) {
        log_error_ast(ast, "Can't get size of non compile time type");
        Expression err = {0};
        return err;
    }

    Expression expression = {0};
    expression.expr_type = et_get_type_size;
    expression.ast = ast;
    expression.get_type_size.expression = alloc(sizeof(Expression));
    *expression.get_type_size.expression = *value;
    expression.type = type_get_number_literal_type();
    return expression;
}

Expression expression_create_member_access(Ast* ast, Scope* scope, Function_Instance* in_function) {
    char* member_name = ast->member_access.member_name;

    massert(ast->type == ast_member_access, "ast is not ast_member_access");
    Expression value = expression_create(ast->member_access.value, scope, in_function);
    if (value.expr_type == et_invalid) return value;
    Type* value_type = &value.type;
    Type_Type value_type_type = type_get_type(value_type);
    Type_Type value_type_type_base = value_type->base->type;

    if (strcmp(member_name, "ptr") == 0 && value_type_type == type_ref) {
        return expression_create_get_ptr(ast, scope, in_function, &value);
    } else if (strcmp(member_name, "val") == 0 && value_type_type == type_ptr) {
        return expression_create_get_val(ast, scope, in_function, &value);
    }
    if (value_type_type == type_ref) {
        Type underlying_type = type_underlying(value_type);
        Type_Type underlying_type_type = type_get_type(&underlying_type);
        if (strcmp(member_name, "val") == 0 && underlying_type_type == type_ptr) {
            Expression deref = expression_cast(&value, &underlying_type);
            return expression_create_get_val(ast, scope, in_function, &deref);
        }
    }

    if (value_type_type == type_compile_time_type) {
        return expression_create_get_type_size(ast, scope, in_function, &value);
    }
    if (value_type_type_base == type_struct) {
        return expression_create_struct_access(ast, scope, in_function, &value);
    } else {
        log_error_ast(ast->member_access.value, "No member to access with name %s", ast->member_access.member_name);
        Expression err = {0};
        return err;
    }
}

Expression expression_create_alloc(Ast* ast, Scope* scope, Function_Instance* in_function) {
    massert(ast->type == ast_alloc, "ast is not ast_alloc");
    Expression expression = {0};
    expression.expr_type = et_alloc;
    expression.ast = ast;
}

Expression expression_create_free(Ast* ast, Scope* scope, Function_Instance* in_function) {
    massert(ast->type == ast_free, "ast is not ast_free");
    massert(false, "not implemented");
}

Expression expression_create_function_call(Ast* ast, Scope* scope, Function_Instance* in_function) {
    massert(ast->type == ast_function_call, "ast is not ast_function_call");
    Expression expression = {0};
    expression.expr_type = et_function_call;
    expression.ast = ast;
    char* function_name = ast->function_call.function_name;
    Expression_List arguments = expression_list_create(ast->function_call.arguments.count);
    for (u64 i = 0; i < ast->function_call.arguments.count; i++) {
        Ast* argument_ast = ast_list_get(&ast->function_call.arguments, i);
        Expression argument = expression_create(argument_ast, scope, in_function);
        if (argument.expr_type == et_invalid) return argument;
        expression_list_add(&arguments, &argument);
    }

    Function_Instance* function_instance = function_find(&arguments, function_name, ast, true);
    if (function_instance == NULL) {
        Expression err = {0};
        return err;
    }
    expression.function_call.function_instance = function_instance;
    for (u64 i = 0; i < arguments.count; i++) {
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

Expression expression_variable_access(Variable* variable) {
    Expression expression = {0};
    expression.expr_type = et_variable;
    expression.ast = NULL;
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

Expression expression_create_type(Ast* ast, Scope* scope, Function_Instance* in_function, bool log_error) {
    Expression expression = {0};
    expression.expr_type = et_type;
    expression.ast = ast;
    expression.type_info.type = type_find_ast(ast, scope, log_error);
    if (expression.type_info.type.base->type == type_invalid) {
        Expression err = {0};
        return err;
    }
    expression.type = type_get_compile_time_type(NULL);
    return expression;
}

Expression expression_create_variable(Ast* ast, Scope* scope, Function_Instance* in_function, bool log_error) {
    Expression expression = {0};
    expression.expr_type = et_variable;
    expression.ast = ast;
    Variable* variable = scope_get_variable(scope, ast->word.word);
    if (variable == NULL) {
        if (log_error) {
            log_error_ast(ast, "Variable %s not found", ast->word.word);
        }
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

Expression expression_create_word(Ast* ast, Scope* scope, Function_Instance* in_function) {
    Expression expression = expression_create_variable(ast, scope, in_function, false);
    if (expression.expr_type != et_invalid) {
        return expression;
    }

    // convert as word to ast type
    Ast type_ast = {0};
    type_ast.type = ast_type;
    type_ast.token = ast->token;
    type_ast.num_tokens = ast->num_tokens;
    type_ast.type_info.is_compile_time_type = false;
    type_ast.type_info.name = ast->word.word;
    type_ast.type_info.modifiers = type_modifier_list_create(0);

    Ast* type_ast_ptr = alloc(sizeof(Ast));
    *type_ast_ptr = type_ast;

    expression = expression_create_type(type_ast_ptr, scope, in_function, false);
    if (expression.expr_type != et_invalid) {
        return expression;
    }

    // call both again to show both errors
    expression_create_variable(ast, scope, in_function, true);
    expression_create_type(type_ast_ptr, scope, in_function, true);

    Expression err = {0};
    return err;
}

Expression expression_create_biop(Ast* ast, Scope* scope, Function_Instance* in_function) {
    massert(false, "not implemented");
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
    if (type_is_invalid(expression)) return true;  // do this to not propagate a billion errors from anything causing an invalid type
    if (type_is_invalid(type)) return true;        // do this to not propagate a billion errors from anything causing an invalid type
    if (type_is_equal(expression, type)) return true;

    Type_Type expression_type_type = type_get_type(expression);
    // Type_Type expression_type_type_base = expression->base->type;
    Type_Type type_type = type_get_type(type);
    // Type_Type type_type_base = type->base->type;
    // Type type_underlying_type = type_underlying(type);

    if (expression_type_type == type_number_literal && type_type == type_int) {
        return true;
    }
    if (expression_type_type == type_number_literal && type_type == type_uint) {
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

    if (expression_type_type == type_ptr) {
        Type expression_type_underlying_type = type_underlying(expression);
        Type_Type expression_type_underlying_type_type = type_get_type(&expression_type_underlying_type);
        if (expression_type_underlying_type_type == type_void && type_type == type_ptr) {
            return true;
        }
    }

    if (type_type == type_ptr) {
        Type type_underlying_type = type_underlying(type);
        Type_Type type_underlying_type_type = type_get_type(&type_underlying_type);
        if (type_underlying_type_type == type_void && expression_type_type == type_ptr) {
            return true;
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

LLVMValueRef expression_compile_get_val(Expression* expression, Function_Instance* func, Scope* scope) {
    return expression_compile(expression->get_val.expression, func, scope);
}

LLVMValueRef expression_compile_get_ptr(Expression* expression, Function_Instance* func, Scope* scope) {
    return expression_compile(expression->get_ptr.expression, func, scope);
}

LLVMValueRef expression_compile_struct_access(Expression* expression, Function_Instance* func, Scope* scope) {
    massert(expression->expr_type == et_struct_access, "expression is not et_struct_access");
    Expression_Struct_Access* struct_access = &expression->struct_access;
    LLVMValueRef value = expression_compile(struct_access->value, func, scope);

    Expression* value_expression = expression->struct_access.value;
    Type* value_type = &value_expression->type;
    Type_Type value_type_type = type_get_type(value_type);
    Type_Base* value_type_base = value_type->base;
    LLVMTypeRef struct_llvm_type = type_get_base_llvm_type(value_type_base);

    if (value_type_type == type_ref && value_type->modifiers.count == 1) {
        LLVMValueRef accessed_value = LLVMBuildStructGEP2(context.llvm_info.builder, struct_llvm_type, value, struct_access->member_index, "struct_access");
        return accessed_value;
    } else if (value_type_type == type_struct) {
        LLVMValueRef accessed_value = LLVMBuildExtractValue(context.llvm_info.builder, value, struct_access->member_index, "struct_access");
        return accessed_value;
    } else {
        massert(false, "should never happen");
    }
}

LLVMValueRef expression_compile_type(Expression* expression, Function_Instance* func, Scope* scope) {
    return NULL;
}

LLVMValueRef expression_compile(Expression* expression, Function_Instance* func, Scope* scope) {
    switch (expression->expr_type) {
        case et_type:
            return expression_compile_type(expression, func, scope);
        case et_get_val:
            return expression_compile_get_val(expression, func, scope);
        case et_get_ptr:
            return expression_compile_get_ptr(expression, func, scope);
        case et_struct_access:
            return expression_compile_struct_access(expression, func, scope);
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
        case et_get_type_size:
            return expression_compile_get_type_size(expression, func, scope);
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

    Type_Type return_type_type = type_get_type(&to_call->type.base->function.return_type);
    if (return_type_type != type_void) {
        LLVMValueRef result = LLVMBuildCall2(context.llvm_info.builder, function_type, function_value, argument_values.data, argument_values.count, "call");
        return result;
    } else {
        LLVMValueRef result = LLVMBuildCall2(context.llvm_info.builder, function_type, function_value, argument_values.data, argument_values.count, "");
        return result;
    }
}

LLVMValueRef expression_compile_get_type_size(Expression* expression, Function_Instance* func, Scope* scope) {
    // do nothing
    return NULL;
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

    if (from_type_type == type_ptr && to_type_type == type_ptr) {
        return LLVMBuildBitCast(context.llvm_info.builder, from_value, to_type_llvm, "ptr_cast");
    }

    massert(false, "not implemented");
}

Expression_Number_Literal expression_get_type_size_number_literal(Expression* expression) {
    Expression_Number_Literal number = {0};
    number.is_float = false;
    Expression* type_expression = expression->get_type_size.expression;
    massert(type_expression->expr_type == et_type, "type_expression is not et_type");
    Type type = type_expression->type_info.type;
    LLVMTypeRef type_llvm = type_get_llvm_type(&type);
    size_t size = LLVMABISizeOfType(context.llvm_info.data_layout, type_llvm);
    u64 number_of_digits = get_number_of_digits(size);
    char* number_string = alloc(number_of_digits + 1);
    sprintf(number_string, "%llu", size);
    number.number = number_string;
    return number;
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
        case et_get_type_size:
            return expression_get_type_size_number_literal(expression);
        case et_function_call:
        case et_type:
        case et_get_val:
        case et_get_ptr:
        case et_struct_access:
        case et_variable:
        case et_multi_expression:
        case et_cast:
        case et_invalid:
            massert(false, "unexpected expression");
    }
}
