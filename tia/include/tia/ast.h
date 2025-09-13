#pragma once
#include "tia/basic.h"
#include "tia/lists.h"
#include "tia/token.h"

typedef struct Ast Ast;
typedef struct Token Token;

typedef enum Ast_Type {
    ast_invalid = 0,
    ast_file,
    ast_function_declaration,
    ast_type,
    ast_variable_declaration,
    ast_return,
    ast_end_statement,
    ast_biop,
    ast_number,
    ast_word,
    ast_string,
    ast_scope,
    ast_assignment,
    ast_multi_expression,
    ast_function_call,
} Ast_Type;

typedef struct Ast_Function_Call {
    char* function_name;
    Ast_List arguments;
} Ast_Function_Call;

typedef struct Ast_Function_Declaration {
    Ast* return_type;
    Ast_List parameters;
    char* name;
    Ast* body;
} Ast_Function_Declaration;

typedef struct Ast_Assignee {
    bool is_variable_declaration;
    union {
        Ast* variable_declaration;
        Ast* expression;
    };
} Ast_Assignee;

typedef struct Ast_Assignment {
    Ast_Assignee_List assignees;
    Ast* value;
} Ast_Assignment;

typedef struct Ast_Variable_Declaration {
    Ast* type;
    char* name;
} Ast_Variable_Declaration;

typedef struct Ast_Type_Info {
    char* name;
    Type_Modifier_List modifiers;
    bool is_compile_time_type;
} Ast_Type_Info;

typedef struct Ast_Return {
    Ast* return_value;
} Ast_Return;

typedef struct Ast_Biop {
    Ast* lhs;
    Ast* rhs;
    Biop_Operator operator;
} Ast_Biop;

typedef struct Ast_Number {
    bool is_float;
    union {
        char* number;
        double number_float;
    };
} Ast_Number;

typedef struct Ast_File {
    Ast_List global_declarations;
} Ast_File;

typedef struct Ast_Word {
    char* word;
} Ast_Word;

typedef struct Ast_String {
    char* string;
} Ast_String;

typedef struct Ast_Scope {
    Ast_List statements;
} Ast_Scope;

typedef struct Ast_Multi_Expression {
    Ast_List expressions;
} Ast_Multi_Expression;

typedef struct Ast {
    Token* token;
    Ast_Type type;
    u64 num_tokens;
    union {
        Ast_Function_Declaration function_declaration;
        Ast_Variable_Declaration variable_declaration;
        Ast_Type_Info type_info;
        Ast_Return return_;
        Ast_Biop biop;
        Ast_Number number;
        Ast_File file;
        Ast_Word word;
        Ast_String string;
        Ast_Scope scope;
        Ast_Assignment assignment;
        Ast_Multi_Expression multi_expression;
        Ast_Function_Call function_call;
    };
} Ast;

Ast ast_create(Token* tokens);

Ast ast_general_parse(Token** tokens);

Ast ast_general_identifier_parse(Token** tokens);

Ast ast_type_parse(Token** tokens);

Ast ast_function_declaration_parse(Token** tokens);

Ast ast_end_statement_parse(Token** tokens);

Ast ast_variable_declaration_parse(Token** tokens);

Ast ast_expresssion_parse(Token** tokens, TokenType_* delimiters, u64 num_delimiters);

Ast ast_word_parse(Token** tokens);

Ast ast_function_call_parse(Token** tokens);

Ast ast_string_parse(Token** tokens);

Ast ast_scope_parse(Token** tokens);

Ast ast_assignee_parse(Token** tokens);

Ast ast_assignment_parse(Token** tokens);

Ast ast_parenthesized_expression_parse(Token** tokens);

#define INVALID_PRECEDENCE INT64_MIN
#define LOWEST_PRECEDENCE INT64_MIN + 1
i64 ast_get_precedence(Biop_Operator token_type);

Ast ast_value_parse(Token** tokens);

Ast ast_return_parse(Token** tokens);

bool ast_parse_as_function_declaration(Token** ast);

bool ast_parse_as_assignment(Token** ast);

bool ast_parseable_as_type(Token** tokens);

bool ast_assignee_parse_as_variable_declaration(Token** ast);
