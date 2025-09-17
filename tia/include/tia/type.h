#pragma once
#include "tia/basic.h"
#include "tia/lists.h"
#include "tia/llvm.h"

typedef struct Type_Base Type_Base;
typedef struct Scope Scope;

typedef enum Type_Modifier_Type {
    type_modifier_invalid = 0,
    type_modifier_ref,
    type_modifier_ptr,
} Type_Modifier_Type;

typedef struct Type_Modifier {
    Type_Modifier_Type type;
    Expression* allocator;
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
    type_template,
    type_struct,
    type_compile_time_type,
    // modifiers
    type_ref,
    type_ptr,
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

typedef struct Type_Struct_Field {
    char* name;
    Type type;
} Type_Struct_Field;

typedef struct Type_Struct {
    Type_Struct_Field_List fields;
} Type_Struct;

typedef struct Type_Function {
    Type return_type;
    Type_List parameters;
} Type_Function;

typedef struct Type_Multi_Value {
    Type_List types;
} Type_Multi_Value;

typedef struct Template_Map {
    char* name;
    Type type;
} Template_Map;

typedef struct Template_To_Type {
    Template_Map_List templates;
} Template_To_Type;

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
        Type_Struct struct_;
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
Type_Base* type_get_template_type_base();
Type type_get_template_type(Ast* ast);
Type_Base* type_get_multi_value_type_base(Type_List* types);
Type type_get_multi_value_type(Type_List* types);
Type_Base* type_get_struct_type_base(Type_List* types, String_List* names, char* name);
Type type_get_struct_type(Type_List* types, String_List* names, char* name);
Type_Base* type_get_compile_time_type_base();
Type type_get_compile_time_type(Ast* ast);

Type_Base* type_prototype_struct(Ast* ast);
void type_implement_struct(Type_Base* ast);

Type_Base* type_find_base_type_without_int_finding(const char* name);
Type_Base* type_find_base_type(const char* name);
Type_Base* type_find_base_type_ast(Ast* ast);

// Type type_find(const char* name);
Type type_find_ast(Ast* ast, Scope* scope, bool log_error);

const char* type_get_template_base_name(Type* type);
char* type_get_name(Type* type);
char* get_function_type_name(Type_List* parameters, Type return_type);
char* get_multi_value_type_name(Type_List* types);
char* type_get_struct_name(const char* last_name, Template_To_Type* template_to_type);

Type_Type type_get_type(Type* type);
Type type_deref(Type* type);       // pointer go to references
Type type_underlying(Type* type);  // pointer go to underlying value
Type type_get_reference(Type* type, Expression* allocator);
Type type_get_ptr(Type* type, Expression* allocator);

Type type_copy(Type* type);
Template_To_Type copy_template_to_type(Template_To_Type* template_to_type);

void type_add_mapping(Template_To_Type* template_to_type, const char* name, Type* type);
Type* type_get_mapping(Template_To_Type* template_to_type, const char* name);
bool type_mapping_equal(Template_To_Type* template_to_type, Template_To_Type* other_template_to_type);
Type type_get_mapped_type(Template_To_Type* template_to_type, Type* type);
bool type_needs_mapped(Template_To_Type* template_to_type, Type* type);

void type_set_allocator(Type* type, Expression* allocator);
void type_set_allocators(Type* type, Type* type_with_allocators);

bool type_should_override_allocators(Type* overriden_type, Type* overriding_type);

bool type_is_equal(Type* type_a, Type* type_b);
bool type_is_equal_without_allocator(Type* type_a, Type* type_b);
bool type_base_is_invalid(Type_Base* type);
bool type_is_invalid(Type* type);
bool type_is_reference_of(Type* ref, Type* of);
bool type_is_template(Type* type);
bool type_is_reference_of_type_type(Type* ref, Type_Type of);
bool type_is_ptr_of_type_type(Type* ref, Type_Type of);

Expression* type_get_allocator(Type* type);

// LLVM compilation
LLVMTypeRef type_get_llvm_type(Type* type);
LLVMTypeRef type_get_base_llvm_type(Type_Base* type_base);
