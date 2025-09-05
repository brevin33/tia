#include "tia.h"
#include "tia/lists.h"
#include "tia/token.h"

Ast ast_create(Token* tokens) {
    Ast ast;
    ast.token = tokens;
    ast.type = ast_file;

    ast.file.global_declarations = ast_list_create(8);

    while (tokens->type != tt_end_of_file) {
        Ast global_declaration = ast_general_parse(&tokens);
        if (global_declaration.type == ast_invalid) {
            // error recover by trying to find the next global statement
            i64 scope_level = 0;
            Token_List open_brace_tokens = token_list_create(2);
            while (tokens->type != tt_end_of_file) {
                if (tokens->type == tt_open_brace) {
                    token_list_add(&open_brace_tokens, tokens);
                    scope_level++;
                } else if (tokens->type == tt_close_brace) {
                    if (scope_level > 0) {
                        token_list_pop(&open_brace_tokens);
                    }
                    scope_level--;
                }
                if (tokens->type == tt_end_statement && scope_level <= 0) {
                    break;
                }
                tokens++;
            }
            for (u64 i = 0; i < open_brace_tokens.count; i++) {
                Token* open_brace_token = token_list_get(&open_brace_tokens, i);
                log_error_token(open_brace_token, "Unclosed open brace");
            }
            continue;
        }
        ast_list_add(&ast.file.global_declarations, &global_declaration);
    }

    ast.num_tokens = tokens - ast.token;
    return ast;
}

bool ast_parse_as_function_declaration(Token** tokens) {
    Token* token = *tokens;
    if (!ast_parseable_as_type(&token)) return false;
    if (token->type != tt_identifier) return false;
    token++;
    if (token->type != tt_open_paren) return false;
    return true;
}

bool ast_parse_as_assignment(Token** tokens) {
    Token* token = *tokens;
    if (ast_parseable_as_type(&token))
        if (token->type == tt_identifier) return true;
    token = *tokens;
    while (token->type != tt_end_statement) {
        if (token->type == tt_equal) return true;
        if (token->type == tt_end_statement) return false;
        token++;
    }
    return false;
}

bool ast_parseable_as_type(Token** tokens) {
    Token* token = *tokens;
    if (token->type != tt_identifier) return false;
    token++;
    // TODO: type modifiers like ptr
    while (true) {
        switch (token->type) {
            case tt_ref:
                token++;
                break;
            default:
                *tokens = token;
                return true;
        }
    }
}

Ast ast_type_parse(Token** tokens) {
    Token* token = *tokens;
    Ast type = {0};
    type.type = ast_type;
    type.token = token;
    type.num_tokens = token - type.token;
    if (token->type != tt_identifier) {
        log_error_token(token, "Expected type name");
        Ast err = {0};
        return err;
    }
    char* type_name = token_get_string(token);
    type.type_info.name = type_name;
    token++;

    Type_Modifier_List modifiers = type_modifier_list_create(2);
    while (true) {
        bool continue_loop = true;
        switch (token->type) {
            case tt_ref: {
                Type_Modifier type_modifier = {0};
                type_modifier.type = type_modifier_ref;
                type_modifier_list_add(&modifiers, &type_modifier);
                token++;
                break;
            }
            default:
                continue_loop = false;
                break;
        }
        if (!continue_loop) break;
    }
    type.type_info.modifiers = modifiers;

    type.num_tokens = token - type.token;
    *tokens = token;
    return type;
}

Ast ast_variable_declaration_parse(Token** tokens) {
    Token* token = *tokens;
    Ast variable_declaration = {0};
    variable_declaration.type = ast_variable_declaration;
    variable_declaration.token = token;

    Ast type = ast_type_parse(&token);
    variable_declaration.variable_declaration.type = alloc(sizeof(Ast));
    *variable_declaration.variable_declaration.type = type;
    if (type.type == ast_invalid) return type;

    if (token->type != tt_identifier) {
        log_error_token(token, "Expected variable name");
        Ast err = {0};
        return err;
    }
    char* variable_name = token_get_string(token);
    variable_declaration.variable_declaration.name = variable_name;
    token++;

    variable_declaration.num_tokens = token - variable_declaration.token;
    *tokens = token;
    return variable_declaration;
}

Ast ast_function_declaration_parse(Token** tokens) {
    Token* token = *tokens;
    Ast function_declaration = {0};
    function_declaration.type = ast_function_declaration;
    function_declaration.token = token;

    Ast type = ast_type_parse(&token);
    function_declaration.function_declaration.return_type = alloc(sizeof(Ast));
    *function_declaration.function_declaration.return_type = type;
    if (type.type == ast_invalid) return type;

    if (token->type != tt_identifier) {
        log_error_token(token, "Expected function name");
        Ast err = {0};
        return err;
    }
    char* function_name = token_get_string(token);
    function_declaration.function_declaration.name = function_name;
    token++;

    if (token->type != tt_open_paren) {
        log_error_token(token, "Expected open paren after function name");
        Ast err = {0};
        return err;
    }
    token++;

    // function parameter parsing
    Ast_List parameters = ast_list_create(2);
    if (token->type != tt_close_paren) {
        while (true) {
            TokenType_ delimiters[] = {tt_comma, tt_close_paren};
            Ast variable_declaration = ast_variable_declaration_parse(&token);
            if (variable_declaration.type == ast_invalid) return variable_declaration;
            ast_list_add(&parameters, &variable_declaration);

            if (token->type == tt_close_paren) {
                break;
            }
            if (token->type == tt_comma) {
                token++;
                continue;
            }
            log_error_token(token, "Expected comma or close paren after function parameter");
            Ast err = {0};
            return err;
        }
    }
    massert(token->type == tt_close_paren, "Expected close paren after function parameter");
    token++;

    function_declaration.function_declaration.parameters = parameters;

    if (token->type == tt_open_brace) {
        Ast scope = ast_scope_parse(&token);
        if (scope.type == ast_invalid) return scope;
        function_declaration.function_declaration.body = alloc(sizeof(Ast));
        *function_declaration.function_declaration.body = scope;
    } else {
        function_declaration.function_declaration.body = NULL;
    }

    function_declaration.num_tokens = token - function_declaration.token;
    *tokens = token;
    return function_declaration;
}

bool ast_assignee_parse_as_variable_declaration(Token** ast) {
    Token* token = *ast;
    if (!ast_parseable_as_type(&token)) return false;
    if (token->type != tt_identifier) return false;
    return true;
}

Ast ast_assignee_parse(Token** tokens) {
    Token* token = *tokens;
    Ast assignee;
    if (ast_assignee_parse_as_variable_declaration(&token)) {
        assignee = ast_variable_declaration_parse(&token);
    } else {
        TokenType_ delimiters[] = {tt_end_statement, tt_equal, tt_comma};
        assignee = ast_expresssion_parse(&token, delimiters, arr_length(delimiters));
    }
    if (assignee.type == ast_invalid) return assignee;
    *tokens = token;
    return assignee;
}

Ast ast_assignment_parse(Token** tokens) {
    Token* token = *tokens;
    Ast ast = {0};
    ast.type = ast_assignment;
    ast.token = token;
    Ast_Assignee_List assignees = ast_assignee_list_create(2);
    while (true) {
        Ast assignee_ast = ast_assignee_parse(&token);
        if (assignee_ast.type == ast_invalid) return assignee_ast;
        Ast_Assignee assignee = {0};
        if (assignee_ast.type == ast_variable_declaration) {
            assignee.is_variable_declaration = true;
        } else {
            assignee.is_variable_declaration = false;
        }
        assignee.variable_declaration = alloc(sizeof(Ast));
        *assignee.variable_declaration = assignee_ast;
        ast_assignee_list_add(&assignees, &assignee);
        if (token->type == tt_equal) break;
        if (token->type == tt_comma) {
            token++;
            continue;
        }
        if (token->type == tt_end_statement) {
            ast.assignment.assignees = assignees;
            ast.num_tokens = token - ast.token;
            ast.assignment.value = NULL;
            *tokens = token;
            return ast;
        }
        log_error_token(token, "Expected equal or comma or end statement");
        Ast err = {0};
        return err;
    }
    massert(token->type == tt_equal, "token type is not tt_equal");
    token++;

    TokenType_ delimiters[] = {tt_end_statement};
    Ast value_ast = ast_expresssion_parse(&token, delimiters, arr_length(delimiters));
    if (value_ast.type == ast_invalid) return value_ast;
    ast.assignment.assignees = assignees;
    ast.assignment.value = alloc(sizeof(Ast));
    *ast.assignment.value = value_ast;
    ast.num_tokens = token - ast.token;
    *tokens = token;
    return ast;
}

Ast ast_scope_parse(Token** tokens) {
    Token* token = *tokens;
    Ast ast = {0};
    ast.type = ast_scope;
    ast.token = token;

    massert(token->type == tt_open_brace, "token type is not tt_open_brace");
    token++;

    ast.scope.statements = ast_list_create(2);
    while (token->type != tt_close_brace) {
        Ast statement = ast_general_parse(&token);
        if (statement.type == ast_invalid) {
            // error recover try and find the next statement
            i64 scope_level = 0;
            while (token->type != tt_end_of_file) {
                if (token->type == tt_open_brace) {
                    scope_level++;
                } else if (token->type == tt_close_brace) {
                    scope_level--;
                    if (scope_level < 0) {
                        break;
                    }
                }
                if (token->type == tt_end_statement && scope_level == 0) {
                    token++;
                    break;
                }
                token++;
            }
            if (token->type == tt_end_of_file) {
                // error will be logged up at global scope as brace not closed
                Ast err = {0};
                return err;
            }
            continue;
        }
        ast_list_add(&ast.scope.statements, &statement);
        if (token->type == tt_end_of_file) {
            Ast err = {0};
            return err;
        }
    }

    massert(token->type == tt_close_brace, "token type is not tt_close_brace");
    token++;

    ast.num_tokens = token - ast.token;
    *tokens = token;
    return ast;
}

Ast ast_general_identifier_parse(Token** tokens) {
    Token* token = *tokens;
    massert(token->type == tt_identifier, "token type is not tt_identifier");
    if (ast_parse_as_function_declaration(&token)) {
        return ast_function_declaration_parse(tokens);
    }
    if (ast_parse_as_assignment(&token)) {
        return ast_assignment_parse(tokens);
    } else {
        TokenType_ delimiters[] = {tt_end_statement};
        return ast_expresssion_parse(tokens, delimiters, arr_length(delimiters));
    }
}

Ast ast_general_parse(Token** tokens) {
    Token* token = *tokens;
    switch (token->type) {
        case tt_identifier:
            return ast_general_identifier_parse(tokens);
        case tt_return:
            return ast_return_parse(tokens);
        case tt_open_brace:
            return ast_scope_parse(tokens);
        case tt_open_paren:
        case tt_open_bracket:
        case tt_number:
        case tt_string: {
            TokenType_ delimiters[] = {tt_end_statement};
            return ast_expresssion_parse(tokens, delimiters, arr_length(delimiters));
        }
        case tt_end_of_file:
        case tt_end_statement:
            return ast_end_statement_parse(tokens);
        case tt_ref:
        case tt_close_brace:
        case tt_close_paren:
        case tt_comma:
        case tt_equal:
        case tt_close_bracket: {
            log_error_token(token, "Unexpected token");
            Ast err = {0};
            return err;
        }
        case tt_invalid: {
            Ast err = {0};
            return err;
        }
    }
}

Ast ast_end_statement_parse(Token** tokens) {
    Token* token = *tokens;
    Ast ast = {0};
    ast.type = ast_end_statement;
    ast.token = token;
    token++;
    ast.num_tokens = token - ast.token;
    *tokens = token;
    return ast;
}

Ast ast_return_parse(Token** tokens) {
    Token* token = *tokens;
    Ast ast = {0};
    ast.type = ast_return;
    ast.token = token;

    massert(token->type == tt_return, "token type is not tt_return");
    token++;

    TokenType_ delimiters[] = {tt_end_statement};
    Ast value = ast_expresssion_parse(&token, delimiters, arr_length(delimiters));
    if (value.type == ast_invalid) return value;
    ast.return_.return_value = alloc(sizeof(Ast));
    *ast.return_.return_value = value;

    ast.num_tokens = token - ast.token;
    *tokens = token;
    return ast;
}

i64 ast_get_precedence(Biop_Operator token_type) {
    switch (token_type) {
        case biop_operator_invalid:
            return INVALID_PRECEDENCE;
    }
}

Ast ast_number_parse(Token** tokens) {
    Token* token = *tokens;
    Ast ast = {0};
    ast.type = ast_number;
    ast.token = token;

    massert(token->type == tt_number, "token type is not tt_number");
    char* number = token_get_string(token);
    if (is_number_float(number)) {
        ast.number.is_float = true;
        ast.number.number_float = string_to_double(number);
    } else {
        ast.number.is_float = false;
        ast.number.number = number;
    }
    token++;

    ast.num_tokens = token - ast.token;
    *tokens = token;
    return ast;
}

Ast ast_function_call_parse(Token** tokens) {
    Token* token = *tokens;
    Ast ast = {0};
    ast.type = ast_function_call;
    ast.token = token;

    massert(token->type == tt_identifier, "token type is not tt_identifier");
    char* function_name = token_get_string(token);
    token++;
    if (token->type != tt_open_paren) {
        log_error_token(token, "Expected open paren after function name");
        Ast err = {0};
        return err;
    }
    token++;

    Ast_Function_Call function_call = {0};
    Ast_List arguments = ast_list_create(0);
    while (true) {
        TokenType_ delimiters[] = {tt_comma, tt_close_paren};
        Ast argument = ast_expresssion_parse(&token, delimiters, arr_length(delimiters));
        if (argument.type == ast_invalid) return argument;
        ast_list_add(&arguments, &argument);
        if (token->type == tt_comma) {
            token++;
            continue;
        }
        if (token->type == tt_close_paren) {
            break;
        }
        log_error_token(token, "Expected comma or close paren after function parameter");
        Ast err = {0};
        return err;
    }
    massert(token->type == tt_close_paren, "token type is not tt_close_paren");
    token++;

    function_call.arguments = arguments;
    function_call.function_name = function_name;
    ast.function_call = function_call;
    ast.num_tokens = token - ast.token;
    *tokens = token;
    return ast;
}

Ast ast_expression_value_id_parse(Token** tokens) {
    Token* token = *tokens;
    massert(token->type == tt_identifier, "token type is not tt_identifier");
    token++;
    if (token->type == tt_open_paren) {
        return ast_function_call_parse(tokens);
    }
    return ast_word_parse(tokens);
}

Ast ast_word_parse(Token** tokens) {
    Token* token = *tokens;
    Ast ast = {0};
    ast.type = ast_word;
    ast.token = token;
    massert(token->type == tt_identifier, "token type is not tt_identifier");
    char* word = token_get_string(token);
    ast.word.word = word;
    token++;
    ast.num_tokens = token - ast.token;
    *tokens = token;
    return ast;
}

Ast ast_string_parse(Token** tokens) {
    Token* token = *tokens;
    Ast ast = {0};
    ast.type = ast_string;
    ast.token = token;
    massert(token->type == tt_string, "token type is not tt_string");
    char* string = token_get_string_content(token);
    ast.string.string = string;
    token++;
    ast.num_tokens = token - ast.token;
    *tokens = token;
    return ast;
}

Ast ast_parenthesized_expression_parse(Token** tokens) {
    Token* token = *tokens;
    Token* start_token = token;
    massert(token->type == tt_open_paren, "token type is not tt_open_paren");
    token++;
    TokenType_ delimiters[] = {tt_close_paren};
    Ast expression = ast_expresssion_parse(&token, delimiters, arr_length(delimiters));
    if (expression.type == ast_invalid) return expression;
    expression.token = start_token;
    expression.num_tokens = token - expression.token;
    *tokens = token;
    return expression;
}

Ast ast_value_parse(Token** tokens) {
    Token* token = *tokens;
    switch (token->type) {
        case tt_number:
            return ast_number_parse(tokens);
        case tt_identifier:
            return ast_expression_value_id_parse(tokens);
        case tt_string:
            return ast_string_parse(tokens);
        case tt_open_paren:
            return ast_parenthesized_expression_parse(tokens);
        case tt_end_of_file:
        case tt_end_statement:
        case tt_close_brace:
        case tt_close_paren:
        case tt_ref:
        case tt_comma:
        case tt_equal:
        case tt_close_bracket:
        case tt_open_bracket:
        case tt_open_brace:
        case tt_return:
        case tt_invalid: {
            log_error_token(token, "Unexpected token can't parse as expression value");
            char* token_type_name = token_get_string(token);
            printf("token type is %s\n", token_type_name);
            Ast err = {0};
            return err;
        }
    }
}

static Ast _ast_expresssion_parse(Token** tokens, TokenType_* delimiters, u64 num_delimiters, i64 precedence) {
    Token* token = *tokens;
    Ast lhs = ast_value_parse(&token);
    if (lhs.type == ast_invalid) return lhs;
    while (true) {
        // check for delimiters
        for (u64 i = 0; i < num_delimiters; i++) {
            TokenType_ delimiter = delimiters[i];
            if (token->type == delimiter) {
                *tokens = token;
                return lhs;
            }
        }

        if (token->type == tt_comma) {
            *tokens = token;
            return lhs;
        }
        Biop_Operator operator = token_get_biop_operator(token);
        i64 operator_precedence = ast_get_precedence(operator);
        if (operator_precedence == INVALID_PRECEDENCE) {
            log_error_token(token, "Expected operator or delimiter");
            Ast err = {0};
            return err;
        }

        if (operator_precedence < precedence) {
            *tokens = token;
            return lhs;
        }

        Ast rhs = ast_value_parse(&token);
        if (rhs.type == ast_invalid) return rhs;

        Ast biop = {0};
        biop.type = ast_biop;
        biop.token = lhs.token;
        biop.biop.lhs = alloc(sizeof(Ast));
        *biop.biop.lhs = lhs;
        biop.biop.rhs = alloc(sizeof(Ast));
        *biop.biop.rhs = rhs;
        biop.biop.operator = operator;
        biop.num_tokens = token - biop.token;

        lhs = biop;
    }
}

Ast ast_expresssion_parse(Token** tokens, TokenType_* delimiters, u64 num_delimiters) {
    bool comma_is_delimiter = false;
    for (u64 i = 0; i < num_delimiters; i++) {
        if (delimiters[i] == tt_comma) {
            comma_is_delimiter = true;
            break;
        }
    }
    if (comma_is_delimiter) {
        return _ast_expresssion_parse(tokens, delimiters, num_delimiters, LOWEST_PRECEDENCE);
    }

    Token* token = *tokens;
    Ast mulit_expression = {0};
    mulit_expression.type = ast_multi_expression;
    mulit_expression.token = token;
    Ast_List expressions = ast_list_create(0);
    while (true) {
        Ast expression_ast = _ast_expresssion_parse(&token, delimiters, num_delimiters, LOWEST_PRECEDENCE);
        if (expression_ast.type == ast_invalid) return expression_ast;
        if (token->type == tt_comma) {
            token++;
            ast_list_add(&expressions, &expression_ast);
        } else if (expressions.count == 0) {
            *tokens = token;
            return expression_ast;
        } else {
            ast_list_add(&expressions, &expression_ast);
            mulit_expression.multi_expression.expressions = expressions;
            mulit_expression.num_tokens = token - mulit_expression.token;
            *tokens = token;
            return mulit_expression;
        }
    }
}
