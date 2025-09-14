#include "tia/function.h"
#include "tia.h"
#include "tia/ast.h"
#include "tia/lists.h"
#include "tia/type.h"

Function* function_new(Ast* ast) {
    massert(ast->type == ast_function_declaration, "ast is not ast_function_declaration");
    Ast_Function_Declaration* function_declaration = &ast->function_declaration;
    Function* function = alloc(sizeof(Function));
    memset(function, 0, sizeof(Function));
    function_pointer_list_add(&context.functions, function);

    function->ast = ast;

    Ast* return_type_ast = function_declaration->return_type;
    Type return_type = type_find_ast(return_type_ast, NULL, true);
    function->return_type = return_type;

    u64 parameter_count = function_declaration->parameters.count;
    Type_List parameters = type_list_create(parameter_count);

    function->is_extern_c = function_declaration->is_extern_c;

    bool is_template_func = false;

    for (u64 i = 0; i < parameter_count; i++) {
        Ast* parameter_ast = ast_list_get(&function_declaration->parameters, i);
        massert(parameter_ast->type == ast_variable_declaration, "parameter_ast is not ast_variable_declaration");

        Ast_Variable_Declaration* parameter_declaration = &parameter_ast->variable_declaration;

        Ast* parameter_type_ast = parameter_declaration->type;
        Type parameter_type = type_find_ast(parameter_type_ast, NULL, true);

        if (parameter_type.base->type == type_template) {
            is_template_func = true;
        } else if (parameter_type.base->type == type_compile_time_type) {
            is_template_func = true;
        }

        type_list_add(&parameters, &parameter_type);

        Variable variable = {0};
        variable.name = parameter_declaration->name;
        variable.type = parameter_type;

        for (u64 i = 0; i < function->parameters.count; i++) {
            Variable* existing = variable_list_get(&function->parameters, i);
            if (strcmp(existing->name, variable.name) == 0) {
                log_error_ast(parameter_ast, "Multiple declarations of parameter with name %s", parameter_declaration->name);
            }
        }
        variable_list_add(&function->parameters, &variable);
    }

    function->name = function_declaration->name;

    if (!is_template_func) {
        function->instances = function_instance_list_create(1);
        Function_Instance function_instance = {0};
        function_instance.function = function;
        Template_To_Type template_to_type = {0};
        function_instance.template_to_type = template_to_type;
        function_instance_list_add(&function->instances, &function_instance);
    }
    return function;
}

void function_prototype_instance(Function_Instance* function_instance) {
    Function* function = function_instance->function;

    function_instance->function = function;
    function_instance->parameters_scope = scope_create(NULL);

    for (u64 i = 0; i < function_instance->template_to_type.templates.count; i++) {
        Template_Map* template_map = template_map_list_get(&function_instance->template_to_type.templates, i);
        char* template_name = template_map->name;
        Type* template_type = &template_map->type;
        Template_Map* templated_map = scope_add_template(&function_instance->parameters_scope, template_name, template_type);
        massert(templated_map != NULL, "templated_map is NULL");
    }

    function_instance->body_scope = scope_create(&function_instance->parameters_scope);

    Type return_type = function->return_type;
    return_type = type_get_mapped_type(&function_instance->template_to_type, &return_type);
    Type_List parameter_types = type_list_create(function->parameters.count);
    for (u64 i = 0; i < function->parameters.count; i++) {
        Variable* variable = variable_list_get(&function->parameters, i);
        Type parameter_type = variable->type;
        parameter_type = type_get_mapped_type(&function_instance->template_to_type, &parameter_type);
        type_list_add(&parameter_types, &parameter_type);
    }
    function_instance->type = type_get_function_type(&parameter_types, return_type);

    for (u64 i = 0; i < function->parameters.count; i++) {
        Type* parameter_type = type_list_get(&parameter_types, i);
        Type_Type parameter_type_type = type_get_type(parameter_type);
        if (parameter_type_type == type_compile_time_type) continue;
        Variable* old_variable = variable_list_get(&function->parameters, i);
        Variable variable = {0};
        variable.name = old_variable->name;
        variable.type = *parameter_type;
        scope_add_variable(&function_instance->parameters_scope, &variable);
    }
}

void function_implement(Function_Instance* function_instance) {
    Function* function = function_instance->function;
    if (function->is_extern_c) return;
    Ast* ast = function->ast;
    Ast* body_ast = ast->function_declaration.body;
    if (body_ast == NULL) return;
    scope_add_statements(&function_instance->body_scope, body_ast, function_instance);
}

void function_llvm_prototype_instance(Function_Instance* function_instance) {
    Function* function = function_instance->function;
    Type* function_type = &function_instance->type;
    LLVMTypeRef function_type_llvm = type_get_llvm_type(function_type);
    char* mangled_name = function_get_mangled_name(function_instance);
    if (strcmp(function->name, "main") == 0) {
        mangled_name = function->name;
    }
    if (function->is_extern_c) {
        mangled_name = function->name;
    }

    LLVMValueRef function_value = LLVMAddFunction(context.llvm_info.module, mangled_name, function_type_llvm);
    function_instance->function_value = function_value;
}

void function_llvm_implement_instance(Function_Instance* function) {
    if (function->function->is_extern_c) return;
    LLVMValueRef function_value = function->function_value;

    LLVMBasicBlockRef entry_block = LLVMAppendBasicBlock(function_value, "entry");
    LLVMPositionBuilderAtEnd(context.llvm_info.builder, entry_block);

    for (u64 i = 0; i < function->parameters_scope.variables.count; i++) {
        Variable* variable = variable_list_get(&function->parameters_scope.variables, i);
        LLVMValueRef param_value = LLVMGetParam(function_value, i);
        Type* variable_type = &variable->type;
        Type_Type variable_type_type = type_get_type(variable_type);
        if (variable_type_type == type_ref) {
            variable->value = param_value;
        } else {
            LLVMTypeRef param_type = type_get_llvm_type(&variable->type);
            if (variable->type.base->type == type_compile_time_type) {
                variable->value = NULL;
            } else {
                variable->value = LLVMBuildAlloca(context.llvm_info.builder, param_type, variable->name);
            }
            LLVMBuildStore(context.llvm_info.builder, param_value, variable->value);
        }
    }

    scope_compile_scope(&function->body_scope, function);
}

Function_Instance* function_find(Expression_List* parameters_exprs, char* name, Ast* ast, bool log_error) {
    Type_List parameters_val = type_list_create(parameters_exprs->count);
    Type_List* parameters = &parameters_val;
    for (u64 i = 0; i < parameters_exprs->count; i++) {
        Expression* parameter_expr = expression_list_get(parameters_exprs, i);
        Type parameter_type = parameter_expr->type;
        type_list_add(parameters, &parameter_type);
    }

    Function_Pointer_List* functions = &context.functions;
    Function_Pointer_List maybe_functions = function_pointer_list_create(8);
    Template_To_Type template_to_type = {0};
    for (u64 i = 0; i < functions->count; i++) {
        Function* function = function_pointer_list_get_function(functions, i);
        if (strcmp(function->name, name) == 0) {
            u64 parameter_count = function->parameters.count;
            if (parameter_count == parameters->count) {
                function_pointer_list_add(&maybe_functions, function);
            }
        }
    }

    // exact match
    Function_Pointer_List match_functions = function_pointer_list_create(8);
    for (u64 i = 0; i < maybe_functions.count; i++) {
        Function* function = function_pointer_list_get_function(&maybe_functions, i);
        bool match = true;
        u64 parameter_count = function->parameters.count;
        for (u64 j = 0; j < parameter_count; j++) {
            Variable* parameter_variable = variable_list_get(&function->parameters, j);
            Type* parameter_type = &parameter_variable->type;
            Type* parameter = type_list_get(parameters, j);
            if (!type_is_equal(parameter, parameter_type)) {
                match = false;
                break;
            }
        }
        if (match) {
            function_pointer_list_add(&match_functions, function);
        }
    }

    if (match_functions.count == 0) {
        // deref match
        for (u64 i = 0; i < maybe_functions.count; i++) {
            Function* function = function_pointer_list_get_function(&maybe_functions, i);
            bool match = true;
            u64 parameter_count = function->parameters.count;
            for (u64 j = 0; j < parameter_count; j++) {
                Variable* parameter_variable = variable_list_get(&function->parameters, j);
                Type* parameter_type = &parameter_variable->type;
                Type* parameter = type_list_get(parameters, j);
                if (type_is_equal(parameter, parameter_type)) {
                    continue;
                }
                Type_Type parameter_type_type = type_get_type(parameter);
                if (parameter_type_type == type_ref) {
                    Type deref_type = type_deref(parameter);
                    if (type_is_equal(&deref_type, parameter_type)) {
                        continue;
                    }
                    match = false;
                    break;
                } else {
                    match = false;
                    break;
                }
            }
            if (match) {
                function_pointer_list_add(&match_functions, function);
            }
        }
        if (match_functions.count == 0) {
            // implicit cast match
            for (u64 i = 0; i < maybe_functions.count; i++) {
                Function* function = function_pointer_list_get_function(&maybe_functions, i);
                bool match = true;
                u64 parameter_count = function->parameters.count;
                for (u64 j = 0; j < parameter_count; j++) {
                    Variable* parameter_variable = variable_list_get(&function->parameters, j);
                    Type* parameter_type = &parameter_variable->type;
                    Type* parameter = type_list_get(parameters, j);
                    bool can_implicit_cast = expression_can_implicitly_cast(parameter, parameter_type);
                    if (!can_implicit_cast) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    function_pointer_list_add(&match_functions, function);
                }
            }
        }

        if (match_functions.count == 0) {
            // template match
            for (u64 i = 0; i < maybe_functions.count; i++) {
                Function* function = function_pointer_list_get_function(&maybe_functions, i);
                bool match = true;
                u64 parameter_count = function->parameters.count;
                Template_To_Type current_template_to_type = {0};
                for (u64 j = 0; j < parameter_count; j++) {
                    Variable* parameter_variable = variable_list_get(&function->parameters, j);
                    Type* parameter_type = &parameter_variable->type;
                    Type* parameter = type_list_get(parameters, j);
                    if (parameter_type->base->type == type_template) {
                        const char* template_name = type_get_template_base_name(parameter_type);
                        Type* mapping = type_get_mapping(&current_template_to_type, template_name);
                        Type_Type expr_parameter_type = type_get_type(parameter);
                        Type_Type func_parameter_type = type_get_type(parameter_type);
                        Type expr_underlying_type = *parameter;
                        if (func_parameter_type == type_ref) {
                            if (expr_parameter_type != type_ref) {
                                match = false;
                                break;
                            }
                        }
                        if (expr_parameter_type == type_ref) {
                            expr_underlying_type = type_deref(parameter);
                        }
                        if (mapping == NULL) {
                            type_add_mapping(&current_template_to_type, template_name, &expr_underlying_type);
                        } else {
                            if (!type_is_equal(&expr_underlying_type, mapping)) {
                                match = false;
                                break;
                            }
                        }
                    } else {
                        bool can_implicit_cast = expression_can_implicitly_cast(parameter, parameter_type);
                        if (!can_implicit_cast) {
                            match = false;
                            break;
                        }
                    }
                }
                if (match) {
                    function_pointer_list_add(&match_functions, function);
                    template_to_type = current_template_to_type;
                }
            }
        }
    }

    if (match_functions.count == 0) {
        char buffer[4192];
        sprintf(buffer, "No function named %s with parameters: ", name);
        for (u64 i = 0; i < parameters->count; i++) {
            char* parameter_name = type_get_name(type_list_get(parameters, i));
            if (i == 0) {
                sprintf(buffer, "%s %s", buffer, parameter_name);
            } else {
                sprintf(buffer, "%s, %s", buffer, parameter_name);
            }
        }
        if (log_error) {
            log_error_ast(ast, buffer);
        }
        return NULL;
    } else if (match_functions.count > 1) {
        char buffer[4192];
        sprintf(buffer, "Multiple functions named %s with parameters: ", name);
        for (u64 i = 0; i < parameters->count; i++) {
            char* parameter_name = type_get_name(type_list_get(parameters, i));
            if (i == 0) {
                sprintf(buffer, "%s %s", buffer, parameter_name);
            } else {
                sprintf(buffer, "%s, %s", buffer, parameter_name);
            }
        }
        if (log_error) {
            log_error_ast(ast, buffer);
            for (u64 i = 0; i < match_functions.count; i++) {
                Function* function = function_pointer_list_get_function(&match_functions, i);
                log_error_ast(function->ast, "Could have meant %s", function->name);
            }
        }
        return NULL;
    }

    Function* function = function_pointer_list_get_function(&match_functions, 0);
    // special handeling for compile time types
    for (u64 i = 0; i < parameters->count; i++) {
        Type* parameter_type = type_list_get(parameters, i);
        Type_Type parameter_type_type = type_get_type(parameter_type);
        char* function_parameter_name = function->parameters.data[i].name;
        if (parameter_type_type == type_compile_time_type) {
            Expression* parameter_expr = expression_list_get(parameters_exprs, i);
            Type parameter_type_info_type = parameter_expr->type_info.type;
            type_add_mapping(&template_to_type, function_parameter_name, &parameter_type_info_type);
        }
    }
    // remove the compile time types form the expression list
    Expression_List new_parameters_exprs = expression_list_create(parameters_exprs->count);
    for (u64 i = 0; i < parameters_exprs->count; i++) {
        Expression* parameter_expr = expression_list_get(parameters_exprs, i);
        Type parameter_type = parameter_expr->type;
        Type_Type parameter_type_type = type_get_type(&parameter_type);
        if (parameter_type_type == type_compile_time_type) {
            continue;
        }
        expression_list_add(&new_parameters_exprs, parameter_expr);
    }
    *parameters_exprs = new_parameters_exprs;

    Function_Instance* function_instance = NULL;  //= &function->instances.data[0];
    for (u64 i = 0; i < function->instances.count; i++) {
        Function_Instance* instance = &function->instances.data[i];
        Template_To_Type* instance_template_to_type = &instance->template_to_type;
        if (type_mapping_equal(instance_template_to_type, &template_to_type)) {
            function_instance = instance;
            break;
        }
    }
    if (function_instance == NULL) {
        Function_Instance function_instance = {0};
        function_instance.function = function;
        function_instance.template_to_type = template_to_type;
        u64 prev_error_count = context.numberOfErrors;
        Function_Instance* added = function_instance_list_add(&function->instances, &function_instance);
        function_prototype_instance(added);
        function_implement(added);
        u64 new_error_count = context.numberOfErrors;
        if (new_error_count > prev_error_count) {
            log_error_ast(ast, "Failed to template function");
            log_error_ast(function->ast, "The function that could not be templated");
        }
        return added;
    }
    return function_instance;
}

char* function_get_mangled_name(Function_Instance* function) {
    Type* function_type = &function->type;
    Type_List* parameters = &function_type->base->function.parameters;
    Type return_type = function_type->base->function.return_type;

    u64 len = 0;
    char* return_type_name = type_get_name(&return_type);
    u64 return_type_name_len = strlen(return_type_name);
    len += return_type_name_len;
    String_List parameter_names = string_list_create(8);
    u64 compile_time_parameter_num = 0;
    for (u64 i = 0; i < parameters->count; i++) {
        Type* parameter = type_list_get(parameters, i);
        char* parameter_name = NULL;
        if (parameter->base->type == type_compile_time_type) {
            Scope* function_scope = &function->parameters_scope;
            for (u64 j = 0; j < function_scope->template_to_type.templates.count; j++) {
                Template_Map* template_map = template_map_list_get(&function_scope->template_to_type.templates, compile_time_parameter_num);
                compile_time_parameter_num++;
                Type* template_type = &template_map->type;
                char* template_name = type_get_name(template_type);
                u64 template_name_len = strlen(template_name);
                parameter_name = alloc(template_name_len + 2);
                parameter_name = alloc(template_name_len + 2);
                parameter_name[0] = '#';
                memcpy(parameter_name + 1, template_name, template_name_len);
                parameter_name[template_name_len + 1] = '\0';
                len += template_name_len + 1;
                break;
            }
            massert(parameter_name != NULL, "parameter_name is NULL");
        } else {
            parameter_name = type_get_name(parameter);
        }
        string_list_add(&parameter_names, parameter_name);
        u64 parameter_name_len = strlen(parameter_name);
        len += parameter_name_len;
    }
    // add extra for spaces
    char* function_name = function->function->name;
    u64 function_name_len = strlen(function_name);
    len += parameters->count + 7 + function_name_len;
    u64 index = 0;
    char* name = alloc(len + 1);
    memcpy(name, return_type_name, return_type_name_len);
    index += return_type_name_len;
    name[index] = '$';
    index++;
    memcpy(name + index, function_name, function_name_len);
    index += function_name_len;
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

Type* function_get_parameter_type(Function_Instance* function, u64 index) {
    Type function_type = function->type;
    massert(function_type.base->type == type_function, "function_type is not type_function");
    Type_Function* function_type_base = &function_type.base->function;
    Type* parameter_type = type_list_get(&function_type_base->parameters, index);
    return parameter_type;
}
