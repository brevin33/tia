#pragma once
#include "tia/basic.h"
#include "tia/lists.h"

typedef enum TokenType_ {
    tt_invalid = 0,
    tt_identifier,
    tt_number,
    tt_string,
    tt_end_of_file,
    tt_end_statement,
    tt_return,
    tt_open_brace,
    tt_close_brace,
    tt_open_paren,
    tt_close_paren,
    tt_open_bracket,
    tt_close_bracket,
    tt_equal,
    tt_comma,
    tt_ref,
    tt_interface,
} TokenType_;

typedef struct Token {
    File* file;
    TokenType_ type;
    u64 start_index;
    u64 end_index;
} Token;

bool token_is_assignment(Token* token);

Token_List token_get_tokens(File* file);
bool token_is_identifier_char(char* c);
bool token_is_number_char(char* c);
bool token_valid_number(Token* c);
void token_swap_if_keyword(Token* token);
char* token_get_string(Token* token);
char* token_get_string_content(Token* token);  // specificly for if token type is tt_string and you don't want the quotes
u64 token_get_lines(Token* token, u64* out_count);
bool token_causes_endline_to_be_endstatement(Token* token);

Biop_Operator token_get_biop_operator(Token* token);
