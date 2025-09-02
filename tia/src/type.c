#include "tia/type.h"
#include <llvm-c/Core.h>
#include "tia.h"
#include "tia/lists.h"

void init_types() {
    // need to kickstart with invalid type so cant call type_base_new with it
    Type_Base* invalid_type_base = alloc(sizeof(Type_Base));
    memset(invalid_type_base, 0, sizeof(Type_Base));
    invalid_type_base->type = type_invalid;
    const char* invalid = "$invalid$";
    u64 invalid_len = strlen(invalid);
    char* invalid_name = alloc(invalid_len + 1);
    memcpy(invalid_name, invalid, invalid_len);

    invalid_name[invalid_len] = '\0';
    invalid_type_base->name = invalid_name;

    type_base_pointer_list_add(&context.types, invalid_type_base);

    type_base_new(type_number_literal, "$Number_Literal$", NULL);
    type_base_new(type_void, "void", NULL);
}

Type_Base* type_base_new(Type_Type type, const char* name, Ast* ast) {
    Type_Base* existing_type = type_find_base_type_without_int_finding(name);
    if (!type_base_is_invalid(existing_type)) {
        log_error_ast(ast, "Type %s already exists", name);
        log_error_ast(existing_type->ast, "Type %s already exists", name);
        return type_get_invalid_type_base();
    }

    Type_Base* type_base = alloc(sizeof(Type_Base));
    memset(type_base, 0, sizeof(Type_Base));
    type_base_pointer_list_add(&context.types, type_base);

    type_base->type = type;
    type_base->name = name;

    return type_base;
}

Type_Base* type_find_base_type_ast(Ast* ast) {
    massert(ast->type == ast_type, "ast is not ast_type");
    char* type_name = ast->type_info.name;

    Type_Base* existing_type = type_find_base_type(type_name);
    if (!type_base_is_invalid(existing_type)) {
        return existing_type;
    }

    Type_Base* invalid_type_base = type_get_invalid_type_base();
    return invalid_type_base;
}

Type_Base* type_get_number_literal_type_base() {
    return type_find_base_type("$Number_Literal$");
}

Type type_get_number_literal_type() {
    Type_Base* type_base = type_get_number_literal_type_base();
    Type type = {0};
    type.base = type_base;
    return type;
}

Type_Base* type_get_int_type_base(u64 bits) {
    char buffer[512];
    snprintf(buffer, 512, "i%llu", bits);
    u64 buffer_len = strlen(buffer);
    Type_Base* existing_type = type_find_base_type_without_int_finding(buffer);
    if (!type_base_is_invalid(existing_type)) {
        return existing_type;
    }
    char* name = alloc(buffer_len + 1);
    memcpy(name, buffer, buffer_len);
    name[buffer_len] = '\0';
    Type_Base* type_base = type_base_new(type_int, name, NULL);
    massert(type_base != NULL, "type_base_new failed");
    type_base->int_.bits = bits;
    return type_base;
}

Type type_get_int_type(u64 bits) {
    Type_Base* type_base = type_get_int_type_base(bits);
    Type type = {0};
    type.base = type_base;
    return type;
}

Type_Base* type_get_uint_type_base(u64 bits) {
    char buffer[512];
    snprintf(buffer, 512, "u%llu", bits);
    u64 buffer_len = strlen(buffer);
    Type_Base* existing_type = type_find_base_type_without_int_finding(buffer);
    if (!type_base_is_invalid(existing_type)) {
        return existing_type;
    }
    char* name = alloc(buffer_len + 1);
    memcpy(name, buffer, buffer_len);
    name[buffer_len] = '\0';
    Type_Base* type_base = type_base_new(type_uint, name, NULL);
    massert(type_base != NULL, "type_base_new failed");
    type_base->uint.bits = bits;
    return type_base;
}

Type type_get_uint_type(u64 bits) {
    Type_Base* type_base = type_get_uint_type_base(bits);
    Type type = {0};
    type.base = type_base;
    return type;
}

Type_Base* type_get_float_type_base(u64 bits) {
    char buffer[512];
    snprintf(buffer, 512, "f%llu", bits);
    u64 buffer_len = strlen(buffer);
    Type_Base* existing_type = type_find_base_type_without_int_finding(buffer);
    if (!type_base_is_invalid(existing_type)) {
        return existing_type;
    }
    char* name = alloc(buffer_len + 1);
    memcpy(name, buffer, buffer_len);
    name[buffer_len] = '\0';
    Type_Base* type_base = type_base_new(type_float, name, NULL);
    massert(type_base != NULL, "type_base_new failed");
    type_base->float_.bits = bits;
    return type_base;
}

Type type_get_float_type(u64 bits) {
    Type_Base* type_base = type_get_float_type_base(bits);
    Type type = {0};
    type.base = type_base;
    return type;
}

Type_Base* type_get_void_type_base() {
    return type_find_base_type("void");
}

Type type_get_void_type() {
    Type_Base* type_base = type_get_void_type_base();
    Type type = {0};
    type.base = type_base;
    return type;
}

Type_Base* type_find_base_type_without_int_finding(const char* name) {
    for (u64 i = 0; i < context.types.count; i++) {
        Type_Base* type = type_base_pointer_list_get_type_base(&context.types, i);
        const char* type_name = type->name;
        if (type_name == NULL) {
            debugbreak();
        }
        if (strcmp(type_name, name) == 0) {
            return type;
        }
    }
    return type_get_invalid_type_base();
}

Type_Base* type_find_base_type(const char* name) {
    Type_Base* type = type_find_base_type_without_int_finding(name);
    if (!type_base_is_invalid(type)) return type;

    // might be a number type
    if (name[0] == 'i') {
        bool error;
        u64 bits = get_string_uint(name + 1, &error);
        if (!error) {
            return type_get_int_type_base(bits);
        }
    }
    if (name[0] == 'u') {
        bool error;
        u64 bits = get_string_uint(name + 1, &error);
        if (!error) {
            return type_get_uint_type_base(bits);
        }
    }
    if (name[0] == 'f') {
        bool error;
        u64 bits = get_string_uint(name + 1, &error);
        if (!error) {
            return type_get_float_type_base(bits);
        }
    }

    return type_get_invalid_type_base();
}

Type type_find(const char* name) {
    Type type = {0};
    Type_Base* type_base = type_find_base_type(name);
    type.base = type_base;
    return type;
}

Type type_find_ast(Ast* ast, bool log_error) {
    massert(ast->type == ast_type, "ast is not ast_type");
    char* name = ast->type_info.name;
    Type type = type_find(name);
    if (type.base->type == type_invalid) {
        if (log_error) {
            log_error_ast(ast, "Type %s not found", name);
        }
    }
    type.ast = ast;
    type.modifiers = ast->type_info.modifiers;
    return type;
}

Type_Base* type_get_function_type_base(Type_List* parameters, Type return_type) {
    char* name = get_function_type_name(parameters, return_type);
    Type_Base* existing_type = type_find_base_type(name);
    if (!type_base_is_invalid(existing_type)) {
        return existing_type;
    }
    Type_Base* type_base = type_base_new(type_function, name, NULL);
    massert(type_base != NULL, "type_base_new failed");
    type_base->function.parameters = *parameters;
    type_base->function.return_type = return_type;
    return type_base;
}

Type type_get_function_type(Type_List* parameters, Type return_type) {
    Type_Base* type_base = type_get_function_type_base(parameters, return_type);
    Type type = {0};
    type.base = type_base;
    return type;
}

Type_Base* type_get_invalid_type_base() {
    Type_Base* type_base = type_base_pointer_list_get_type_base(&context.types, 0);
    return type_base;
}

Type type_get_invalid_type() {
    Type_Base* type_base = type_get_invalid_type_base();
    Type type = {0};
    type.base = type_base;
    return type;
}

Type_Base* type_get_multi_value_type_base(Type_List* types) {
    char* name = get_multi_value_type_name(types);
    Type_Base* existing_type = type_find_base_type(name);
    if (!type_base_is_invalid(existing_type)) {
        return existing_type;
    }
    Type_Base* type_base = type_base_new(type_multi_value, name, NULL);
    massert(type_base != NULL, "type_base_new failed");
    type_base->multi_value.types = *types;
    return type_base;
}

Type type_get_multi_value_type(Type_List* types) {
    Type_Base* type_base = type_get_multi_value_type_base(types);
    Type type = {0};
    type.base = type_base;
    return type;
}

char* type_get_name(Type* type) {
    u64 len = strlen(type->base->name);
    char* name = alloc(len + 1);
    memcpy(name, type->base->name, len);
    name[len] = '\0';
    return name;
}

char* get_multi_value_type_name(Type_List* types) {
    u64 len = 0;
    String_List type_names = string_list_create(8);
    for (u64 i = 0; i < types->count; i++) {
        Type* type = type_list_get(types, i);
        char* type_name = type_get_name(type);
        string_list_add(&type_names, type_name);
        u64 type_name_len = strlen(type_name);
        len += type_name_len;
    }
    len += types->count - 1;
    char* name = alloc(len + 1);
    u64 index = 0;
    for (u64 i = 0; i < types->count; i++) {
        char* type_name = string_list_get_string(&type_names, i);
        u64 type_name_len = strlen(type_name);
        memcpy(name + index, type_name, type_name_len);
        index += type_name_len;
        if (i < types->count - 1) {
            name[index] = ',';
            index++;
        }
    }
    return name;
}

char* get_function_type_name(Type_List* parameters, Type return_type) {
    u64 len = 0;
    char* return_type_name = type_get_name(&return_type);
    u64 return_type_name_len = strlen(return_type_name);
    len += return_type_name_len;
    String_List parameter_names = string_list_create(8);
    for (u64 i = 0; i < parameters->count; i++) {
        Type* parameter = type_list_get(parameters, i);
        char* parameter_name = type_get_name(parameter);
        string_list_add(&parameter_names, parameter_name);
        u64 parameter_name_len = strlen(parameter_name);
        len += parameter_name_len;
    }

    // add extra for spaces and fn
    len += parameters->count + 7;
    u64 index = 0;
    char* name = alloc(len + 1);
    memcpy(name, return_type_name, return_type_name_len);
    index += return_type_name_len;
    name[index] = '$';
    index++;
    name[index] = 'f';
    index++;
    name[index] = 'n';
    index++;
    name[index] = '(';
    index++;
    for (u64 i = 0; i < parameters->count; i++) {
        char* parameter_name = string_list_get_string(&parameter_names, i);
        u64 parameter_name_len = strlen(parameter_name);
        memcpy(name + index, parameter_name, parameter_name_len);
        index += parameter_name_len;
        if (i < parameters->count - 1) {
            name[index] = ',';
            index++;
        }
    }
    name[index] = ')';
    index++;
    name[index] = '\0';
    return name;
}

Type_Type type_get_type(Type* type) {
    if (type->modifiers.count > 0) {
        Type_Modifier* last_modifier = type_modifier_list_get(&type->modifiers, type->modifiers.count - 1);
        switch (last_modifier->type) {
            case type_modifier_ref:
                return type_ref;
            case type_modifier_invalid:
                return type_invalid;
        }
    }
    return type->base->type;
}

Type type_deref(Type* type) {  // pointer go to references
    Type_Type type_type = type_get_type(type);
    massert(type_type == type_ref, "type is not type_ref");
    switch (type_type) {
        case type_ref: {
            Type deref_type = *type;
            deref_type.modifiers.count--;
            return deref_type;
        }
        default:
            return type_get_invalid_type();
    }
}

Type type_get_real_type(Type* type, Type_Substitution_List* substitutions) {
    Type_Type type_type = type->base->type;
    switch (type_type) {
        case type_int:
        case type_float:
        case type_uint: {
            for (u64 i = 0; i < substitutions->count; i++) {
                Type_Substitution* substitution = type_substitution_list_get(substitutions, i);
                Type_Base* base_type = type->base;
                Type_Base* substituted_type = substitution->substituted_type;
                if (base_type == substituted_type) {
                    Type new_type;
                    new_type.modifiers = type_modifier_list_create(8);
                    massert(type->modifiers.count == 0, "type->modifiers.count != 0");
                    for (u64 j = 0; j < substitution->new_type.modifiers.count; j++) {
                        Type_Modifier* modifier = type_modifier_list_get(&substitution->new_type.modifiers, j);
                        type_modifier_list_add(&new_type.modifiers, modifier);
                    }
                    new_type.ast = type->ast;
                    new_type.base = substitution->new_type.base;
                    return new_type;
                }
            }
            return *type;
        }
        case type_ref: {
            Type deref_type = type_deref(type);
            deref_type = type_get_real_type(&deref_type, substitutions);
            return type_get_reference(&deref_type);
        }
        case type_function: {
            Type_List new_parameters = type_list_create(type->base->function.parameters.count);
            for (u64 i = 0; i < type->base->function.parameters.count; i++) {
                Type* parameter = type_list_get(&type->base->function.parameters, i);
                Type new_parameter = type_get_real_type(parameter, substitutions);
                type_list_add(&new_parameters, &new_parameter);
            }
            Type new_return_type = type_get_real_type(&type->base->function.return_type, substitutions);
            Type_Base* new_Function_base_type = type_get_function_type_base(&new_parameters, new_return_type);
            Type new_type;
            new_type.ast = type->ast;
            new_type.base = new_Function_base_type;
            new_type.modifiers = type_modifier_list_create(0);
            return new_type;
        }
        case type_multi_value: {
            Type_List new_types = type_list_create(type->base->multi_value.types.count);
            for (u64 i = 0; i < type->base->multi_value.types.count; i++) {
                Type* type = type_list_get(&type->base->multi_value.types, i);
                Type new_type = type_get_real_type(type, substitutions);
                type_list_add(&new_types, &new_type);
            }
            Type_Base* new_MultiValue_base_type = type_get_multi_value_type_base(&new_types);
            Type new_type;
            new_type.ast = type->ast;
            new_type.base = new_MultiValue_base_type;
            new_type.modifiers = type_modifier_list_create(0);
            return new_type;
        }
        case type_number_literal:
        case type_invalid:
        case type_void: {
            // can't substitute these type
            return *type;
        }
    }
}

Type type_get_reference(Type* type) {
    Type type_copy = *type;
    Type_Modifier ref = {0};
    ref.type = type_modifier_ref;
    type_modifier_list_add(&type_copy.modifiers, &ref);
    return type_copy;
}

Type type_underlying(Type* type) {  // pointer go to underlying value
    Type_Type type_type = type_get_type(type);
    massert(type_type == type_ref, "type is not type_ref");
    switch (type_type) {
        case type_ref: {
            Type deref_type = *type;
            deref_type.modifiers.count--;
            return deref_type;
        }
        default:
            return type_get_invalid_type();
    }
}

bool type_is_reference_of(Type* ref, Type* of) {
    Type_Type ref_type = type_get_type(ref);
    if (ref_type != type_ref) return false;
    Type ref_deref = type_deref(ref);
    return type_is_equal(&ref_deref, of);
}

bool type_substitutions_is_equal(Type_Substitution_List* substitutions_a, Type_Substitution_List* substitutions_b) {
    if (substitutions_a->count != substitutions_b->count) return false;
    for (u64 i = 0; i < substitutions_a->count; i++) {
        Type_Substitution* substitution_a = type_substitution_list_get(substitutions_a, i);
        bool found = false;
        for (u64 j = 0; j < substitutions_b->count; j++) {
            Type_Substitution* substitution_b = type_substitution_list_get(substitutions_b, j);
            if (substitution_a->substituted_type == substitution_b->substituted_type) {
                if (type_is_equal(&substitution_a->new_type, &substitution_b->new_type)) {
                    found = true;
                    break;
                }
            }
        }
        if (!found) return false;
    }
    return true;
}

bool type_is_equal(Type* type_a, Type* type_b) {
    if (type_a->base->type != type_b->base->type) return false;
    if (type_a->modifiers.count != type_b->modifiers.count) return false;
    for (u64 i = 0; i < type_a->modifiers.count; i++) {
        Type_Modifier* modifier_a = type_modifier_list_get(&type_a->modifiers, i);
        Type_Modifier* modifier_b = type_modifier_list_get(&type_b->modifiers, i);
        if (modifier_a->type != modifier_b->type) return false;
    }
    return true;
}

bool type_base_is_invalid(Type_Base* type) {
    return type->type == type_invalid;
}

bool type_is_invalid(Type* type) {
    return type_base_is_invalid(type->base);
}

LLVMTypeRef type_get_llvm_type(Type* type, Type_Substitution_List* substitutions) {
    Type real_type = type_get_real_type(type, substitutions);
    return type_get_llvm_type_no_substitution(&real_type);
}

LLVMTypeRef type_get_llvm_type_no_substitution(Type* type) {
    if (type->modifiers.count > 0) {
        Type_Modifier* last_modifier = type_modifier_list_get(&type->modifiers, type->modifiers.count - 1);
        switch (last_modifier->type) {
            case type_modifier_ref: {
                Type deref_type = type_deref(type);
                LLVMTypeRef deref_llvm_type = type_get_llvm_type_no_substitution(&deref_type);
                return LLVMPointerType(deref_llvm_type, 0);
            }
            case type_modifier_invalid:
                massert(false, "invalid type");
                return NULL;
        }
    }
    return type_get_base_llvm_type_no_substitution(type->base);
}

LLVMTypeRef type_get_base_llvm_type_no_substitution(Type_Base* type_base) {
    Type_Type type_type = type_base->type;
    switch (type_type) {
        case type_int:
            return LLVMIntType(type_base->int_.bits);
        case type_float:
            if (type_base->float_.bits == 32) {
                return LLVMFloatType();
            } else if (type_base->float_.bits == 64) {
                return LLVMDoubleType();
            } else if (type_base->float_.bits == 16) {
                return LLVMHalfType();
            } else {
                red_printf("invalid float bits: %u\n", type_base->float_.bits);
                massert(false, "invalid float bits");
                abort();
            }
            return NULL;
        case type_uint:
            return LLVMIntType(type_base->uint.bits);
            break;
        case type_void:
            return LLVMVoidType();
            break;
        case type_function: {
            LLVMTypeRef return_llvm_type = type_get_llvm_type_no_substitution(&type_base->function.return_type);
            LLVMTypeRef parameter_type_buffer[512];
            if (type_base->function.parameters.count > 512) {
                red_printf("too many parameters\n");
                abort();
            }
            for (u64 i = 0; i < type_base->function.parameters.count; i++) {
                Type* parameter_type = type_list_get(&type_base->function.parameters, i);
                parameter_type_buffer[i] = type_get_llvm_type_no_substitution(parameter_type);
            }
            return LLVMFunctionType(return_llvm_type, parameter_type_buffer, type_base->function.parameters.count, false);
            break;
        }
        case type_invalid:
        case type_ref:
        case type_number_literal:
        case type_multi_value:
            return NULL;
            break;
    }
}
