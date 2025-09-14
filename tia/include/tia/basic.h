#pragma once
#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "stdarg.h"
#include "assert.h"
#include "math.h"

#ifdef _WIN32
#define debugbreak() __debugbreak()
#else
#define debugbreak()
#endif

#ifdef NDEBUG
#define massert(condition, message)
#else
#define massert(condition, message)                    \
    if (!(condition)) {                                \
        red_printf("Assertion failed: %s\n", message); \
        debugbreak();                                  \
        abort();                                       \
    }
#endif

#define arr_length(arr) (sizeof(arr) / sizeof(arr[0]))

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

typedef enum Biop_Operator {
    biop_operator_invalid = 0,
    biop_operator_multiply,
} Biop_Operator;

void red_printf(const char* format, ...);

void green_printf(const char* format, ...);

void* alloc(size_t size);  // uses the tia_context arena to allocate memory

char* make_portable_slashes(const char* path);

char* read_file(const char* path);

char* get_file_extension(const char* path);

char* get_file_name(const char* path);

u64 get_string_uint(const char* string, bool* out_error);

u64 get_number_of_digits(u64 number);

char** get_directory_in_directory(const char* path_, u64* out_count);

char** get_file_in_directory(const char* path_, u64* out_count);

bool is_number_float(char* c);

double string_to_double(char* string);

bool directory_exists(const char* path);

bool file_exists(const char* path);

bool make_directory(const char* path);

bool delete_file(const char* path);

bool delete_directory(const char* path);

bool link_obj_to_exe(const char* obj_path, const char* exe_path);

#define CASE_NUMBER \
    case '0':       \
    case '1':       \
    case '2':       \
    case '3':       \
    case '4':       \
    case '5':       \
    case '6':       \
    case '7':       \
    case '8':       \
    case '9'

#define CASE_LETTER \
    case 'a':       \
    case 'b':       \
    case 'c':       \
    case 'd':       \
    case 'e':       \
    case 'f':       \
    case 'g':       \
    case 'h':       \
    case 'i':       \
    case 'j':       \
    case 'k':       \
    case 'l':       \
    case 'm':       \
    case 'n':       \
    case 'o':       \
    case 'p':       \
    case 'q':       \
    case 'r':       \
    case 's':       \
    case 't':       \
    case 'u':       \
    case 'v':       \
    case 'w':       \
    case 'x':       \
    case 'y':       \
    case 'z':       \
    case 'A':       \
    case 'B':       \
    case 'C':       \
    case 'D':       \
    case 'E':       \
    case 'F':       \
    case 'G':       \
    case 'H':       \
    case 'I':       \
    case 'J':       \
    case 'K':       \
    case 'L':       \
    case 'M':       \
    case 'N':       \
    case 'O':       \
    case 'P':       \
    case 'Q':       \
    case 'R':       \
    case 'S':       \
    case 'T':       \
    case 'U':       \
    case 'V':       \
    case 'W':       \
    case 'X':       \
    case 'Y':       \
    case 'Z'
