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
    type_base_new(type_template, "$Template$", NULL);
    type_base_new(type_compile_time_type, "type", NULL);
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
    type_base->ast = ast;

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

// Type type_find(const char* name) {
//     Type type = {0};
//     Type_Base* type_base = type_find_base_type(name);
//     type.base = type_base;
//     return type;
// }

Type type_find_ast(Ast* ast, Scope* scope, bool log_error) {
    massert(ast->type == ast_type, "ast is not ast_type");
    char* name = ast->type_info.name;

    if (scope != NULL) {
        Type* templated_type = scope_get_templated_type(scope, name);
        if (templated_type != NULL) {
            return *templated_type;
        }
    }

    Type_Base* type_base;
    if (ast->type_info.is_compile_time_type) {
        // special compile time types
        if (strcmp(name, "type") == 0) {
            return type_get_compile_time_type(ast);
        }

        // default templating
        type_base = type_get_template_type_base();
    } else {
        type_base = type_find_base_type_ast(ast);
    }
    if (type_base->type == type_invalid) {
        if (log_error) {
            char* name = ast->type_info.name;
            log_error_ast(ast, "Type %s not found", name);
        }
    }
    Type type = {0};
    type.ast = ast;
    type.modifiers = ast->type_info.modifiers;
    type.base = type_base;

    Type_Type base_type_type = type_base->type;
    if (base_type_type == type_template && scope != NULL) {
        log_error_ast(ast, "template does not have a mapping");
        return type_get_invalid_type();
    }

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

Type_Base* type_get_template_type_base() {
    return type_find_base_type("$Template$");
}

Type_Base* type_get_compile_time_type_base() {
    return type_find_base_type("type");
}

Type type_get_compile_time_type(Ast* ast) {
    Type_Base* type_base = type_get_compile_time_type_base();
    Type type = {0};
    type.base = type_base;
    type.ast = ast;
    return type;
}

Type type_get_template_type(Ast* ast) {
    Type_Base* type_base = type_get_template_type_base();
    Type type = {0};
    type.base = type_base;
    type.ast = ast;
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

Type_Base* type_get_struct_type_base(Type_List* types, String_List* names, char* name) {
    Type_Base* existing_type = type_find_base_type(name);
    if (!type_base_is_invalid(existing_type)) {
        return existing_type;
    }

    Type_Base* type_base = type_base_new(type_struct, name, NULL);
    massert(type_base != NULL, "type_base_new failed");

    Type_Struct_Field_List fields = type_struct_field_list_create(types->count);
    for (u64 i = 0; i < types->count; i++) {
        Type* type = type_list_get(types, i);
        Type_Struct_Field field = {0};
        field.name = string_list_get_string(names, i);
        field.type = *type;
        type_struct_field_list_add(&fields, &field);
    }
    type_base->struct_.fields = fields;
    return type_base;
}

Type type_get_struct_type(Type_List* types, String_List* names, char* name) {
    Type_Base* type_base = type_get_struct_type_base(types, names, name);
    Type type = {0};
    type.base = type_base;
    type.ast = NULL;
    return type;
}

Type_Base* type_prototype_struct(Ast* ast) {
    massert(ast->type == ast_struct_declaration, "ast is not ast_struct_declaration");

    return type_base_new(type_struct, ast->struct_declaration.name, ast);
}

void type_implement_struct(Type_Base* base) {
    Ast* ast = base->ast;
    base->struct_.fields = type_struct_field_list_create(ast->struct_declaration.fields.count);
    for (u64 i = 0; i < ast->struct_declaration.fields.count; i++) {
        Ast* field = ast_list_get(&ast->struct_declaration.fields, i);
        massert(field->type == ast_variable_declaration, "field is not ast_variable_declaration");
        Type_Struct_Field struct_field = {0};
        struct_field.name = field->variable_declaration.name;
        struct_field.type = type_find_ast(field->variable_declaration.type, NULL, true);
        if (struct_field.type.base->type == type_invalid) {
            // do nothing
        }

        for (u64 i = 0; i < base->struct_.fields.count; i++) {
            Type_Struct_Field* existing_field = type_struct_field_list_get(&base->struct_.fields, i);
            char* existing_field_name = existing_field->name;
            if (strcmp(existing_field_name, struct_field.name) == 0) {
                log_error_ast(field, "Field %s already exists", existing_field_name);
            }
        }

        type_struct_field_list_add(&base->struct_.fields, &struct_field);
    }
}

Type type_get_multi_value_type(Type_List* types) {
    Type_Base* type_base = type_get_multi_value_type_base(types);
    Type type = {0};
    type.base = type_base;
    return type;
}

const char* type_get_template_base_name(Type* type) {
    massert(type->base->type == type_template, "type is not type_template");
    Ast* ast = type->ast;
    char* name = ast->type_info.name;
    return name;
}

char* type_get_name(Type* type) {
    const char* base_name = NULL;
    Type_Type type_type = type->base->type;
    if (type_type == type_template) {
        base_name = type_get_template_base_name(type);
    } else {
        base_name = type->base->name;
    }

    u64 len = strlen(base_name);
    for (u64 i = 0; i < type->modifiers.count; i++) {
        switch (type->modifiers.data[i].type) {
            case type_modifier_ref:
                len += 1;
                break;
            case type_modifier_ptr:
                len += 1;
                break;
            case type_modifier_invalid:
                break;
        }
    }
    char* name = alloc(len + 1);
    memcpy(name, base_name, len);
    u64 count = strlen(base_name);
    for (u64 i = 0; i < type->modifiers.count; i++) {
        switch (type->modifiers.data[i].type) {
            case type_modifier_ref: {
                name[count] = '&';
                count++;
                break;
            }
            case type_modifier_ptr: {
                name[count] = '*';
                count++;
                break;
            }
            case type_modifier_invalid:
                break;
        }
    }
    name[count] = '\0';
    return name;
}

char* type_get_struct_name(const char* last_name, Template_To_Type* template_to_type) {
    u64 last_name_len = strlen(last_name);
    u64 len = strlen(last_name) + 2;
    for (u64 i = 0; i < template_to_type->templates.count; i++) {
        Template_Map* template_map = template_map_list_get(&template_to_type->templates, i);
        char* template_name = template_map->name;
        u64 template_name_len = strlen(template_name);
        len += template_name_len + 1;
        Type* template_type = &template_map->type;
        char* template_type_name = type_get_name(template_type);
        u64 template_type_name_len = strlen(template_type_name);
        len += template_type_name_len + 1;
    }
    char* name = alloc(len + 1);
    u64 index = 0;
    memcpy(name, last_name, last_name_len);
    name[last_name_len] = '(';
    index += last_name_len + 1;
    for (u64 i = 0; i < template_to_type->templates.count; i++) {
        Template_Map* template_map = template_map_list_get(&template_to_type->templates, i);
        char* template_name = template_map->name;
        u64 template_name_len = strlen(template_name);
        memcpy(name + index, template_name, template_name_len);
        index += template_name_len;
        name[index] = '=';
        index++;
        Type* template_type = &template_map->type;
        char* template_type_name = type_get_name(template_type);
        u64 template_type_name_len = strlen(template_type_name);
        memcpy(name + index, template_type_name, template_type_name_len);
        index += template_type_name_len;
        if (i < template_to_type->templates.count - 1) {
            name[index] = ',';
            index++;
        }
    }
    name[index] = ')';
    index++;
    name[index] = '\0';
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
            case type_modifier_ptr:
                return type_ptr;
            case type_modifier_invalid:
                return type_invalid;
        }
    }
    return type->base->type;
}

Type type_deref(Type* type) {  // pointer go to references
    Type_Type type_type = type_get_type(type);
    switch (type_type) {
        case type_ref: {
            Type deref_type = type_copy(type);
            deref_type.modifiers.count--;
            Type_Type deref_type_type = type_get_type(&deref_type);
            massert(deref_type_type != type_ref, "type is type_ref after deref");
            return deref_type;
        }
        case type_ptr: {
            Type deref_type = type_copy(type);
            Type_Modifier* last_modifier = type_modifier_list_get(&deref_type.modifiers, deref_type.modifiers.count - 1);
            last_modifier->type = type_modifier_ref;
            return deref_type;
        }
        case type_invalid:
        case type_number_literal:
        case type_void:
        case type_compile_time_type:
        case type_function:
        case type_multi_value:
        case type_template:
        case type_struct:
        case type_int:
        case type_float:
        case type_uint:
            massert(false, "unexpected type");
            return type_get_invalid_type();
    }
}

void type_add_mapping(Template_To_Type* template_to_type, const char* name, Type* type) {
    Template_Map template_map = {0};
    u64 len = strlen(name) + 1;
    template_map.name = alloc(len + 1);
    memcpy(template_map.name, name, len);
    template_map.type = *type;
    template_map_list_add(&template_to_type->templates, &template_map);
}

bool type_needs_mapped(Template_To_Type* template_to_type, Type* type) {
    Type_Type type_type = type_get_type(type);
    switch (type_type) {
        case type_struct: {
            Type_Struct* struct_ = &type->base->struct_;
            for (u64 i = 0; i < struct_->fields.count; i++) {
                Type_Struct_Field* field = type_struct_field_list_get(&struct_->fields, i);
                Type* field_type = &field->type;
                if (type_needs_mapped(template_to_type, field_type)) {
                    return true;
                }
            }
            return false;
        }
        case type_ptr:
        case type_ref: {
            Type deref_type = type_deref(type);
            return type_needs_mapped(template_to_type, &deref_type);
        }
        case type_multi_value: {
            Type_List* types = &type->base->multi_value.types;
            for (u64 i = 0; i < types->count; i++) {
                Type* type = type_list_get(types, i);
                if (type_needs_mapped(template_to_type, type)) {
                    return true;
                }
            }
            return false;
        }
        case type_function: {
            Type return_type = type->base->function.return_type;
            if (type_needs_mapped(template_to_type, &return_type)) {
                return true;
            }
            Type_List* parameters = &type->base->function.parameters;
            for (u64 i = 0; i < parameters->count; i++) {
                Type* parameter = type_list_get(parameters, i);
                if (type_needs_mapped(template_to_type, parameter)) {
                    return true;
                }
            }
            return false;
        }
        case type_invalid:
        case type_int:
        case type_float:
        case type_compile_time_type:
        case type_uint:
        case type_number_literal:
        case type_void:
            return false;
        case type_template:
            return true;
    }
}

Template_To_Type copy_template_to_type(Template_To_Type* template_to_type) {
    Template_To_Type template_to_type_copy = {0};
    for (u64 i = 0; i < template_to_type->templates.count; i++) {
        Template_Map* template_map = template_map_list_get(&template_to_type->templates, i);
        Template_Map template_map_copy;
        template_map_copy.name = template_map->name;
        template_map_copy.type = type_copy(&template_map->type);
        template_map_list_add(&template_to_type_copy.templates, &template_map_copy);
    }
    return template_to_type_copy;
}

Type type_copy(Type* type) {
    Type type_copy = *type;
    type_copy.modifiers = type_modifier_list_create(type->modifiers.count);
    for (u64 i = 0; i < type->modifiers.count; i++) {
        Type_Modifier* modifier = type_modifier_list_get(&type->modifiers, i);
        Type_Modifier modifier_copy = *modifier;
        type_modifier_list_add(&type_copy.modifiers, &modifier_copy);
    }
    return type_copy;
}

Type type_get_ptr(Type* type) {
    Type copy = type_copy(type);
    Type_Modifier ptr = {0};
    ptr.type = type_modifier_ptr;
    type_modifier_list_add(&copy.modifiers, &ptr);
    return copy;
}

Type type_get_mapped_type(Template_To_Type* template_to_type, Type* type) {
    if (!type_needs_mapped(template_to_type, type)) {
        return *type;
    }
    Type_Type type_type = type_get_type(type);
    switch (type_type) {
        case type_struct: {
            Type_Struct* struct_ = &type->base->struct_;
            Type_List new_type_list = type_list_create(struct_->fields.count);
            String_List new_type_names = string_list_create(struct_->fields.count);
            for (u64 i = 0; i < struct_->fields.count; i++) {
                Type_Struct_Field* field = type_struct_field_list_get(&struct_->fields, i);
                Type* field_type = &field->type;
                Type field_type_mapped = type_get_mapped_type(template_to_type, field_type);
                char* field_name = field->name;
                type_list_add(&new_type_list, &field_type_mapped);
                string_list_add(&new_type_names, field_name);
            }
            char* new_name = type_get_struct_name(type->base->name, template_to_type);
            return type_get_struct_type(&new_type_list, &new_type_names, new_name);
        }
        case type_template: {
            const char* name = type_get_template_base_name(type);
            Type* to_map_to = type_get_mapping(template_to_type, name);
            if (to_map_to == NULL) {
                log_error_ast(type->ast, "Template not declared in function declaration: %s", name);
                return type_get_invalid_type();
            }
            return *to_map_to;
        }
        case type_ptr: {
            Type deref_type = type_deref(type);
            Type deref_mapped_type = type_get_mapped_type(template_to_type, &deref_type);
            return type_get_ptr(&deref_mapped_type);
        }
        case type_ref: {
            Type deref_type = type_deref(type);
            Type deref_mapped_type = type_get_mapped_type(template_to_type, &deref_type);
            return type_get_reference(&deref_mapped_type);
        }
        case type_multi_value: {
            Type_List* types = &type->base->multi_value.types;
            Type_List mapped_types = type_list_create(types->count);
            for (u64 i = 0; i < types->count; i++) {
                Type* type = type_list_get(types, i);
                Type mapped_type = type_get_mapped_type(template_to_type, type);
                type_list_add(&mapped_types, &mapped_type);
            }
            Type multi_value_type = type_get_multi_value_type(&mapped_types);
            return multi_value_type;
        }
        case type_function: {
            Type_List* parameters = &type->base->function.parameters;
            Type_List mapped_parameters = type_list_create(parameters->count);
            for (u64 i = 0; i < parameters->count; i++) {
                Type* parameter = type_list_get(parameters, i);
                Type mapped_parameter = type_get_mapped_type(template_to_type, parameter);
                type_list_add(&mapped_parameters, &mapped_parameter);
            }
            Type return_type = type_get_mapped_type(template_to_type, &type->base->function.return_type);
            return type_get_function_type(&mapped_parameters, return_type);
        }
        case type_invalid:
        case type_int:
        case type_compile_time_type:
        case type_float:
        case type_uint:
        case type_void:
        case type_number_literal:
            return *type;
    }
}

Type* type_get_mapping(Template_To_Type* template_to_type, const char* name) {
    for (u64 i = 0; i < template_to_type->templates.count; i++) {
        Template_Map* template_map = template_map_list_get(&template_to_type->templates, i);
        if (strcmp(template_map->name, name) == 0) {
            return &template_map->type;
        }
    }
    return NULL;
}

bool type_mapping_equal(Template_To_Type* template_to_type, Template_To_Type* other_template_to_type) {
    if (template_to_type->templates.count != other_template_to_type->templates.count) return false;
    for (u64 i = 0; i < template_to_type->templates.count; i++) {
        Template_Map* template_map = template_map_list_get(&template_to_type->templates, i);
        char* name1 = template_map->name;
        bool found = false;
        for (u64 j = 0; j < other_template_to_type->templates.count; j++) {
            Template_Map* other_template_map = template_map_list_get(&other_template_to_type->templates, j);
            char* name2 = other_template_map->name;
            if (strcmp(name1, name2) == 0 && type_is_equal(&template_map->type, &other_template_map->type)) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

Type type_get_reference(Type* type) {
    Type copy = type_copy(type);
    Type_Modifier ref = {0};
    ref.type = type_modifier_ref;
    type_modifier_list_add(&copy.modifiers, &ref);
    return copy;
}

Type type_underlying(Type* type) {  // pointer go to underlying value
    Type_Type type_type = type_get_type(type);
    switch (type_type) {
        case type_ptr:
        case type_ref: {
            Type deref_type = *type;
            deref_type.modifiers.count--;
            return deref_type;
        }
        case type_invalid:
        case type_number_literal:
        case type_void:
        case type_compile_time_type:
        case type_function:
        case type_multi_value:
        case type_template:
        case type_struct:
        case type_int:
        case type_float:
        case type_uint:
            massert(false, "unexpected type");
            return type_get_invalid_type();
    }
}

bool type_is_reference_of_type_type(Type* ref, Type_Type of) {
    Type ref_deref = type_deref(ref);
    return type_get_type(&ref_deref) == of;
}

bool type_is_ptr_of_type_type(Type* ref, Type_Type of) {
    Type underlying_type = type_underlying(ref);
    return type_get_type(&underlying_type) == of;
}

bool type_is_template(Type* type) {
    Type_Type type_type = type_get_type(type);
    return type_type == type_template;
}

bool type_is_reference_of(Type* ref, Type* of) {
    Type_Type ref_type = type_get_type(ref);
    if (ref_type != type_ref) return false;
    Type ref_deref = type_deref(ref);
    return type_is_equal(&ref_deref, of);
}

bool type_is_equal(Type* type_a, Type* type_b) {
    if (type_a->base != type_b->base) return false;
    if (type_a->modifiers.count != type_b->modifiers.count) return false;
    for (u64 i = 0; i < type_a->modifiers.count; i++) {
        Type_Modifier* modifier_a = type_modifier_list_get(&type_a->modifiers, i);
        Type_Modifier* modifier_b = type_modifier_list_get(&type_b->modifiers, i);
        if (modifier_a->type != modifier_b->type) return false;
    }
    Type_Type type_a_type = type_a->base->type;
    Type_Type type_b_type = type_b->base->type;
    if (type_a_type == type_template && type_b_type == type_template) {
        const char* name_a = type_get_template_base_name(type_a);
        const char* name_b = type_get_template_base_name(type_b);
        if (strcmp(name_a, name_b) != 0) {
            return false;
        }
    }
    return true;
}

bool type_base_is_invalid(Type_Base* type) {
    return type->type == type_invalid;
}

bool type_is_invalid(Type* type) {
    return type_base_is_invalid(type->base);
}

LLVMTypeRef type_get_llvm_type(Type* type) {
    if (type->modifiers.count > 0) {
        Type_Modifier* last_modifier = type_modifier_list_get(&type->modifiers, type->modifiers.count - 1);
        switch (last_modifier->type) {
            case type_modifier_ptr: {
                Type underlying_type = type_underlying(type);
                Type_Type underlying_type_type = type_get_type(&underlying_type);
                LLVMTypeRef underlying_llvm_type = type_get_llvm_type(&underlying_type);
                if (underlying_type_type == type_void) {
                    return LLVMPointerType(LLVMIntType(8), 0);
                }
                return LLVMPointerType(underlying_llvm_type, 0);
            }
            case type_modifier_ref: {
                Type deref_type = type_deref(type);
                LLVMTypeRef deref_llvm_type = type_get_llvm_type(&deref_type);
                return LLVMPointerType(deref_llvm_type, 0);
            }
            case type_modifier_invalid:
                massert(false, "invalid type");
                return NULL;
        }
    }
    return type_get_base_llvm_type(type->base);
}

LLVMTypeRef type_get_base_llvm_type(Type_Base* type_base) {
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
            LLVMTypeRef return_llvm_type = type_get_llvm_type(&type_base->function.return_type);
            LLVMTypeRef parameter_type_buffer[512];
            if (type_base->function.parameters.count > 512) {
                red_printf("too many parameters\n");
                abort();
            }
            u64 count = 0;
            for (u64 i = 0; i < type_base->function.parameters.count; i++) {
                Type* parameter_type = type_list_get(&type_base->function.parameters, i);
                Type_Type parameter_type_type = type_get_type(parameter_type);
                if (parameter_type_type == type_compile_time_type) continue;
                parameter_type_buffer[count] = type_get_llvm_type(parameter_type);
                count++;
            }
            return LLVMFunctionType(return_llvm_type, parameter_type_buffer, count, false);
            break;
        }
        case type_struct: {
            LLVMTypeRef field_types[512];
            if (type_base->struct_.fields.count > 512) {
                red_printf("too many fields\n");
                abort();
            }
            for (u64 i = 0; i < type_base->struct_.fields.count; i++) {
                Type_Struct_Field* field = type_struct_field_list_get(&type_base->struct_.fields, i);
                Type* field_type = &field->type;
                field_types[i] = type_get_llvm_type(field_type);
            }
            return LLVMStructType(field_types, type_base->struct_.fields.count, false);
        }
        case type_invalid:
        case type_compile_time_type:
        case type_ref:
        case type_ptr:
        case type_number_literal:
        case type_multi_value:
        case type_template:
            return NULL;
            break;
    }
}
