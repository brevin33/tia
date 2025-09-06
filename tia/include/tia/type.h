#pragma once
#include "tia/basic.h"
#include "tia/lists.h"
#include "tia/llvm.h"

typedef struct Type_Base Type_Base;

typedef enum Type_Modifier_Type {
    type_modifier_invalid = 0,
    type_modifier_ref,
} Type_Modifier_Type;

typedef struct Type_Modifier {
    Type_Modifier_Type type;
} Type_Modifier;

typedef struct Type {
    Ast* ast;
    Type_Base* base;
    Type_Modifier_List modifiers;
} Type;

typedef enum Type_Type {
    // base types
    type_invalid = 0,
    type_int,
    type_float,
    type_uint,
    type_number_literal,
    type_void,
    type_function,
    type_multi_value,
    type_interface,
    // modifiers
    type_ref,
} Type_Type;

typedef struct Type_Int {
    u64 bits;
} Type_Int;

typedef struct Type_Float {
    u64 bits;
} Type_Float;

typedef struct Type_Uint {
    u64 bits;
} Type_Uint;

typedef struct Type_Function {
    Type return_type;
    Type_List parameters;
} Type_Function;

typedef struct Type_Substitution {
    Type_Base* substituted_type;
    Type new_type;
} Type_Substitution;

typedef struct Type_Multi_Value {
    Type_List types;
} Type_Multi_Value;

typedef struct Type_Interface_Function {
    Type function_type;
    char* function_name;
} Type_Interface_Function;

typedef struct Type_Interface {
    Type_Interface_Function_List functions;
} Type_Interface;

typedef struct Type_Base {
    Type_Type type;
    const char* name;
    Ast* ast;
    union {
        Type_Int int_;
        Type_Float float_;
        Type_Uint uint;
        Type_Function function;
        Type_Multi_Value multi_value;
        Type_Interface interface;
    };
} Type_Base;

void init_types();

Type_Base* type_base_new(Type_Type type, const char* name, Ast* ast);

Type_Base* type_get_number_literal_type_base();
Type type_get_number_literal_type();
Type_Base* type_get_int_type_base(u64 bits);
Type type_get_int_type(u64 bits);
Type_Base* type_get_uint_type_base(u64 bits);
Type type_get_uint_type(u64 bits);
Type_Base* type_get_float_type_base(u64 bits);
Type type_get_float_type(u64 bits);
Type_Base* type_get_void_type_base();
Type type_get_void_type();
Type_Base* type_get_function_type_base(Type_List* parameters, Type return_type);
Type type_get_function_type(Type_List* parameters, Type return_type);
Type_Base* type_get_invalid_type_base();
Type type_get_invalid_type();
Type_Base* type_get_multi_value_type_base(Type_List* types);
Type type_get_multi_value_type(Type_List* types);

Type_Base* type_prototype_interface(Ast* ast);
void type_implement_interface(Type_Base* type_base);

Type_Base* type_find_base_type_without_int_finding(const char* name);
Type_Base* type_find_base_type(const char* name);
Type_Base* type_find_base_type_ast(Ast* ast);
Type type_find(const char* name);
Type type_find_ast(Ast* ast, bool log_error);

char* type_get_name(Type* type);
char* get_function_type_name(Type_List* parameters, Type return_type);
char* get_multi_value_type_name(Type_List* types);

Type_Type type_get_type(Type* type);
Type type_deref(Type* type);       // pointer go to references
Type type_underlying(Type* type);  // pointer go to underlying value
Type type_get_reference(Type* type);
Type type_get_real_type(Type* type, Type_Substitution_List* substitutions);

bool type_is_equal(Type* type_a, Type* type_b);
bool type_base_is_invalid(Type_Base* type);
bool type_is_invalid(Type* type);
bool type_substitutions_is_equal(Type_Substitution_List* substitutions_a, Type_Substitution_List* substitutions_b);
bool type_is_reference_of(Type* ref, Type* of);

//returns the function return type
Type type_interface_is_there_a_interface_function(Type_List* arguments, char* function_name);

bool type_fullfills_interface(Type* type, Type* interface);

// LLVM compilation
LLVMTypeRef type_get_llvm_type(Type* type, Type_Substitution_List* substitutions);
LLVMTypeRef type_get_llvm_type_no_substitution(Type* type);
LLVMTypeRef type_get_base_llvm_type_no_substitution(Type_Base* type_base);
