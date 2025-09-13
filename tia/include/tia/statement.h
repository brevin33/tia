#pragma once
#include "tia/expression.h"
#include "tia/lists.h"
#include "tia/scope.h"

typedef struct Function Function;
typedef struct Scope Scope;
typedef struct Expression Expression;

typedef enum Statement_Type {
    st_invalid = 0,
    st_return,
    st_ignore,
    st_assignment,
    st_expression,
    st_scope,
} Statement_Type;

typedef struct Statement_Return {
    Expression return_value;
} Statement_Return;

typedef struct Statement_Expression {
    Expression expression;
} Statement_Expression;

typedef struct Assignee {
    bool is_variable_declaration;
    union {
        Variable* variable;
        Expression* expression;
    };
} Assignee;

typedef struct Statement_Assignment {
    Assignees_List assignees;
    Expression* value;
} Statement_Assignment;

typedef struct Statement_variable_declaration {
    Variable* variable;
} Statement_variable_declaration;

typedef struct Statement_Scope {
    Scope scope;
} Statement_Scope;

typedef struct Statement {
    Statement_Type type;
    Ast* ast;
    union {
        Statement_Return return_;
        Statement_variable_declaration variable_declaration;
        Statement_Expression expression;
        Statement_Scope scope;
        Statement_Assignment assignment;
    };
} Statement;

Statement statement_create(Ast* ast, Scope* scope, Function_Instance* function);

Statement statement_create_return(Ast* ast, Scope* scope, Function_Instance* function);

Statement statement_create_assignment(Ast* ast, Scope* scope, Function_Instance* function);

Statement statement_create_ignore(Ast* ast, Scope* scope, Function_Instance* function);

Statement statement_create_expression(Ast* ast, Scope* scope, Function_Instance* function);

Statement statement_create_scope(Ast* ast, Scope* scope, Function_Instance* function);

Type* statement_get_assignee_type(Assignee* assignee);

bool statement_compile(Statement* statement, Function_Instance* func, Scope* scope);

bool statement_compile_return(Statement* statement, Function_Instance* func, Scope* scope);

bool statement_compile_assignment(Statement* statement, Function_Instance* func, Scope* scope);

bool statement_compile_expression(Statement* statement, Function_Instance* func, Scope* scope);

bool statement_compile_scope(Statement* statement, Function_Instance* func, Scope* scope);
