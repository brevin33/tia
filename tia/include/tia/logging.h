#pragma once

typedef struct Token Token;
void log_error_token(Token* token, const char* message, ...);

typedef struct Ast Ast;
void log_error_ast(Ast* ast, const char* message, ...);

void log_error(const char* message, ...);
