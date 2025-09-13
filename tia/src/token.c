#include "tia.h"

bool token_is_assignment(Token* token) {
    return token->type == tt_equal;
}

bool token_is_number_char(char* c) {
    switch (*c) {
        case '.':
        case '_':
        CASE_NUMBER:
            return true;
        default:
            return false;
    }
}

bool token_is_identifier_char(char* c) {
    switch (*c) {
        case '_':
        CASE_LETTER:
        CASE_NUMBER:
            return true;
        default:
            return false;
    }
}

bool token_is_string_char(char* c) {
    switch (*c) {
        case '\0':
        case '\n':
        case '"':
            return false;
        default:
            return true;
    }
}

// specificly for if token type is tt_string and you don't want the quotes
char* token_get_string_content(Token* token) {
    massert(token->type == tt_string, "token type is not tt_string");
    char* str = token_get_string(token);
    str = str + 1;
    u64 len = strlen(str);
    str[len - 1] = '\0';
    return str;
}

char* token_get_string(Token* token) {
    File* file = token->file;
    char* file_contents = file->contents;
    u64 size = token->end_index - token->start_index;
    char* string = alloc(size + 1);
    memcpy(string, &file_contents[token->start_index], size);
    string[size] = '\0';
    return string;
}

bool token_valid_number(Token* c) {
    char* file_contents = c->file->contents;
    bool has_decimal = false;
    for (u64 i = c->start_index; i < c->end_index; i++) {
        char* c = &file_contents[i];
        if (*c == '.') {
            if (has_decimal) {
                return false;
            }
            has_decimal = true;
        }
    }
    return true;
}

void token_swap_if_keyword(Token* token) {
    char* string = token_get_string(token);
    if (strcmp(string, "return") == 0) {
        token->type = tt_return;
    }
}

Token_List token_get_tokens(File* file) {
    char* file_contents = file->contents;
    Token_List tokens = token_list_create(8);
    u64 index = 0;
    while (file_contents[index] != '\0') {
        Token token;
        token.start_index = index;
        token.file = file;
        switch (file_contents[index]) {
            case '\t':
            case '\r':
            case ' ': {
                index++;
                continue;
            }
            CASE_NUMBER: {
                token.type = tt_number;
                while (token_is_number_char(&file_contents[index])) index++;
                token.end_index = index;  // need to complete this early so
                if (!token_valid_number(&token)) {
                    token.type = tt_invalid;
                    log_error_token(&token, "Invalid number");
                }
                break;
            }
            case '_':
            CASE_LETTER: {
                token.type = tt_identifier;
                while (token_is_identifier_char(&file_contents[index])) index++;
                token.end_index = index;  // need to complete this early so swap if keyword can get the token string
                token_swap_if_keyword(&token);
                break;
            }
            case '"': {
                token.type = tt_string;
                index++;
                while (token_is_string_char(&file_contents[index])) index++;
                if (file_contents[index] != '"') {
                    token.type = tt_invalid;
                    token.end_index = index;
                    log_error_token(&token, "Unterminated string");
                }
                break;
            }
            case '\n': {
                index++;
                Token* last_token = token_list_get(&tokens, tokens.count - 1);
                if (token_causes_endline_to_be_endstatement(last_token)) {
                    token.type = tt_end_statement;
                } else {
                    continue;
                }
                break;
            }
            case ';': {
                index++;
                token.type = tt_end_statement;
                break;
            }
            case '\0': {
                token.type = tt_end_of_file;
                break;
            }
            case '{': {
                index++;
                token.type = tt_open_brace;
                break;
            }
            case '}': {
                index++;
                token.type = tt_close_brace;
                break;
            }
            case '(': {
                index++;
                token.type = tt_open_paren;
                break;
            }
            case ')': {
                index++;
                token.type = tt_close_paren;
                break;
            }
            case '[': {
                index++;
                token.type = tt_open_bracket;
                break;
            }
            case ']': {
                index++;
                token.type = tt_close_bracket;
                break;
            }
            case ',': {
                index++;
                token.type = tt_comma;
                break;
            }
            case '=': {
                index++;
                token.type = tt_equal;
                break;
            }
            case '&': {
                index++;
                token.type = tt_ref;
                break;
            }
            case '$': {
                index++;
                token.type = tt_dollar;
                break;
            }
            case '#': {
                index++;
                token.type = tt_hash;
                break;
            }
            default: {
                index++;
                token.type = tt_invalid;
                token.end_index = index;
                log_error_token(&token, "Invalid character");
            }
        }
        u64 end_index = index;
        token.end_index = end_index;
        token_list_add(&tokens, &token);
    }
    Token end_of_file_token;
    end_of_file_token.type = tt_end_of_file;
    end_of_file_token.start_index = index;
    end_of_file_token.end_index = index;
    token_list_add(&tokens, &end_of_file_token);

    return tokens;
}

bool token_causes_endline_to_be_endstatement(Token* token) {
    switch (token->type) {
        case tt_identifier:
        case tt_number:
        case tt_string:
        case tt_invalid:
        case tt_return:
        case tt_close_brace:
        case tt_close_paren:
        case tt_close_bracket:
            return true;
        case tt_dollar:
        case tt_hash:
        case tt_ref:
        case tt_end_statement:
        case tt_end_of_file:
        case tt_open_brace:
        case tt_open_paren:
        case tt_open_bracket:
        case tt_comma:
        case tt_equal:
            return false;
    }
}

Biop_Operator token_get_biop_operator(Token* token) {
    switch (token->type) {
        default:
            return biop_operator_invalid;
    }
}

u64 token_get_lines(Token* token, u64* out_count) {
    return file_get_lines(token->file, token->start_index, token->end_index, out_count);
}
