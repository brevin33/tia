#pragma once
#include "tia/lists.h"
#include "tia/type.h"
#include "tia/ast.h"
#include "tia/function.h"

typedef struct Scope Scope;

typedef enum Expression_Type {
    et_invalid = 0,
    et_number_literal,
    et_variable,
    et_biop,
    et_multi_expression,
    et_cast,
    et_function_call,
} Expression_Type;

typedef struct Expression_Number {
    bool is_float;
    union {
        char* number;
        double number_float;
    };
} Expression_Number;

typedef struct Expression_Function_Call {
    Function_Instance* function_instance;
    Expression_List arguments;
} Expression_Function_Call;

typedef struct Expression_Multi_Expression {
    Expression_List expressions;
} Expression_Multi_Expression;

typedef struct Expression_Variable {
    Variable* variable;
} Expression_Variable;

typedef struct Expression_Biop {
    Expression* lhs;
    Expression* rhs;
    Biop_Operator operator;
} Expression_Biop;

typedef struct Expression_Cast {
    Expression* expression;
} Expression_Cast;

typedef struct Expression {
    Expression_Type expr_type;
    Type type;
    Ast* ast;
    union {
        Expression_Number number;
        Expression_Multi_Expression multi_expression;
        Expression_Variable variable;
        Expression_Biop biop;
        Expression_Cast cast;

        Expression_Function_Call function_call;
    };
} Expression;

typedef struct Expression_To_LLVM_Value {
    Expression* expression;
    LLVMValueRef value;
} Expression_To_LLVM_Value;

Expression expression_create(Ast* ast, Scope* scope, Function_Instance* in_function);

Expression expression_create_number_literal(Ast* ast, Scope* scope, Function_Instance* in_function);

Expression expression_create_variable(Ast* ast, Scope* scope, Function_Instance* in_function);

Expression expression_create_biop(Ast* ast, Scope* scope, Function_Instance* in_function);

Expression expression_create_word(Ast* ast, Scope* scope, Function_Instance* in_function);

Expression expression_create_multi_expression(Ast* ast, Scope* scope, Function_Instance* in_function);

Expression expression_create_function_call(Ast* ast, Scope* scope, Function_Instance* in_function);

bool expression_can_implicitly_cast_without_deref(Type* expression, Type* type);

bool expression_can_implicitly_cast(Type* expression, Type* type);

Expression expression_implicitly_cast(Expression* expression, Type* type);

Expression expression_cast(Expression* expression, Type* type);

LLVMValueRef expression_compile(Expression* expression, Function_Instance* func, Scope* scope);

LLVMValueRef expression_compile_number_literal(Expression* expression, Function_Instance* func, Scope* scope);

LLVMValueRef expression_compile_variable(Expression* expression, Function_Instance* func, Scope* scope);

LLVMValueRef expression_compile_cast(Expression* expression, Function_Instance* func, Scope* scope);

LLVMValueRef expression_compile_function_call(Expression* expression, Function_Instance* func, Scope* scope);

// multi value is special and actually return a pointer to LLVMValueRef array so we need to cast it.
LLVMValueRef expression_compile_multi_expression(Expression* expression, Function_Instance* func, Scope* scope);

typedef struct Expression_Number_Literal {
    bool is_float;
    union {
        char* number;
        double number_float;
    };
} Expression_Number_Literal;

Expression_Number_Literal expression_get_number_literal(Expression* expression);

Expression_Number_Literal expression_get_number_literal_number_literal(Expression* expression);
