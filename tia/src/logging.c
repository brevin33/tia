#include <ctype.h>
#include "tia.h"

static void log_error_base(const char* message, File* file, u64 line, u64 lines, u64 start_index, u64 end_index, va_list args) {
    char* file_contents = file->contents;
    char* file_path = file->path;

    if (lines == 1) {
        red_printf("Error in %s on line %llu: ", file_path, line);
    } else {
        red_printf("Error in %s on lines %llu-%llu: ", file_path, line, lines + line - 1);
    }
    printf("\x1b[31m");
    vprintf(message, args);
    printf("\x1b[0m");
    printf("\n");

    for (u64 i = 0; i < lines; i++) {
        u64 l = line + i;
        char* line = file_get_line(file, l);
        while (isspace(*line)) line++;  // skip whitespace
        printf("%6llu | %s\n", l, line);

        u64 line_start_index = u64_list_get_u64(&file->line_start_indexes, l);
        u64 index = line_start_index;

        printf("%6llu | ", l);
        while (isspace(file_contents[index])) index++;  // skip whitespace
        while (file_contents[index] != '\n' && file_contents[index] != '\0') {
            if (start_index <= index && index < end_index) {
                printf("^");
            } else {
                printf(" ");
            }
            index++;
        }
        printf("\n");
    }
}

static void log_error_v(const char* message, va_list args) {
    red_printf("Error: ");
    vprintf(message, args);
    printf("\n");
}

void log_error(const char* message, ...) {
    va_list args;
    va_start(args, message);
    red_printf("Error: ");
    vprintf(message, args);
    va_end(args);
    printf("\n");
}

void log_error_ast(Ast* ast, const char* message, ...) {
    va_list args;
    va_start(args, message);

    if (ast == NULL) {
        log_error_v(message, args);
        return;
    }

    Token* token = ast->token;
    u64 num_tokens = ast->num_tokens;
    u64 start_index = token->start_index;
    u64 end_index = token[num_tokens - 1].end_index;
    u64 lines;
    u64 line = file_get_lines(token->file, start_index, end_index, &lines);
    log_error_base(message, token->file, line, lines, start_index, end_index, args);

    va_end(args);
}

void log_error_token(Token* token, const char* message, ...) {
    u64 lines;
    u64 line = token_get_lines(token, &lines);
    va_list args;
    va_start(args, message);
    log_error_base(message, token->file, line, lines, token->start_index, token->end_index, args);
    va_end(args);
}
