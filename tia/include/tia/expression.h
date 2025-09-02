#pragma once
#include "tia/lists.h"
#include "tia/type.h"
#include "tia/ast.h"

typedef struct Scope Scope;

typedef enum Expression_Type {
    et_invalid = 0,
    et_number_literal,
    et_variable,
    et_biop,
    et_multi_expression,
    et_cast,
} Expression_Type;

typedef struct Expression_Number {
    bool is_float;
    union {
        char* number;
        double number_float;
    };
} Expression_Number;

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
    };
    union {
        LLVMValueRef value;
    };
} Expression;

Expression expression_create(Ast* ast, Scope* scope);

Expression expression_create_number_literal(Ast* ast, Scope* scope);

Expression expression_create_variable(Ast* ast, Scope* scope);

Expression expression_create_biop(Ast* ast, Scope* scope);

Expression expression_create_word(Ast* ast, Scope* scope);

Expression expression_create_multi_expression(Ast* ast, Scope* scope);

bool expression_can_implicitly_cast_without_deref(Type* expression, Type* type);

bool expression_can_implicitly_cast(Type* expression, Type* type);

Expression expression_implicitly_cast(Expression* expression, Type* type);

Expression expression_cast(Expression* expression, Type* type);

void expression_compile(Expression* expression, Function* func, Scope* scope, Type_Substitution_List* substitutions, Variable_LLVM_Value_List* var_to_llvm_val, LLVMValueRef function_value);

void expression_compile_number_literal(Expression* expression, Function* func, Scope* scope, Type_Substitution_List* substitutions, Variable_LLVM_Value_List* var_to_llvm_val, LLVMValueRef function_value);

void expression_compile_variable(Expression* expression, Function* func, Scope* scope, Type_Substitution_List* substitutions, Variable_LLVM_Value_List* var_to_llvm_val, LLVMValueRef function_value);

void expression_compile_cast(Expression* expression, Function* func, Scope* scope, Type_Substitution_List* substitutions, Variable_LLVM_Value_List* var_to_llvm_val, LLVMValueRef function_value);

typedef struct Expression_Number_Literal {
    bool is_float;
    union {
        char* number;
        double number_float;
    };
} Expression_Number_Literal;

Expression_Number_Literal expression_get_number_literal(Expression* expression);

Expression_Number_Literal expression_get_number_literal_number_literal(Expression* expression);
