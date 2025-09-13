#include "tia.h"

Context context = {0};

static void context_init(Arena* arena) {
    context.arena = arena;
    context.folders = folder_pointer_list_create(8);
    context.files = file_pointer_list_create(8);
    context.types = type_base_pointer_list_create(8);
    context.functions = function_pointer_list_create(8);
    context.numberOfErrors = 0;
    init_types();
}

Folder* create_tia_project(char* path, Arena* arena) {
    context_init(arena);
    Folder* folder = folder_new(path);

    if (context.numberOfErrors > 0) {
        red_printf("Failed to parse project has errors\n");
        return NULL;
    } else {
        green_printf("Successfully parsed project\n");
    }

    return folder;
}

char* compile_tia_project(Folder* folder) {
    if (context.numberOfErrors > 0) {
        red_printf("Can't compile project with errors\n");
        red_printf("Number of errors: %u\n", context.numberOfErrors);
        return NULL;
    }
    LLVMInitializeAllAsmParsers();
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmPrinters();
    LLVMInitializeAllDisassemblers();
    char* triple = LLVMGetDefaultTargetTriple();

    context.llvm_info.llvm_context = LLVMContextCreate();
    context.llvm_info.module = LLVMModuleCreateWithName(folder->name);
    context.llvm_info.builder = LLVMCreateBuilder();

    char* error;
    LLVMTargetRef target;
    if (LLVMGetTargetFromTriple(triple, &target, &error) != 0) {
        fprintf(stderr, "Failed to get target: %s\n", error);
        LLVMDisposeMessage(error);
        return NULL;
    }

    LLVMTargetMachineRef targetMachine;
    targetMachine = LLVMCreateTargetMachine(target, triple, "", "", LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault);

    context.llvm_info.data_layout = LLVMCreateTargetDataLayout(targetMachine);

    // compile functions that need compiled
    for (u64 i = 0; i < context.functions.count; i++) {
        Function* function = function_pointer_list_get_function(&context.functions, i);
        for (u64 j = 0; j < function->instances.count; j++) {
            Function_Instance* function_instance = &function->instances.data[j];
            function_llvm_prototype_instance(function_instance);
        }
    }

    // compile functions that need compiled
    for (u64 i = 0; i < context.functions.count; i++) {
        Function* function = function_pointer_list_get_function(&context.functions, i);
        for (u64 j = 0; j < function->instances.count; j++) {
            Function_Instance* function_instance = &function->instances.data[j];
            function_llvm_implement_instance(function_instance);
        }
    }

    char* ir = LLVMPrintModuleToString(context.llvm_info.module);
    printf("\n%s\n\n", ir);
    LLVMDisposeMessage(ir);

    if (LLVMVerifyModule(context.llvm_info.module, LLVMAbortProcessAction, &error) != 0) {
        red_printf("LLVMVerifyModule failed: %s\n", error);
        LLVMDisposeMessage(error);
        return NULL;
    } else {
        green_printf("Successfully verified module\n");
    }

    u64 path_len = strlen(folder->path);
    u64 build_dir_len = path_len + 6;
    char* build_dir = alloc(build_dir_len + 1);
    memcpy(build_dir, folder->path, path_len);
    build_dir[path_len] = '/';
    memcpy(build_dir + path_len + 1, "build", 5);
    build_dir[build_dir_len] = '\0';

    // make dir if it doesn't exist
    make_directory(build_dir);

    u64 obj_path_len = build_dir_len + 8;
    char* obj_path = alloc(obj_path_len + 1);
    memcpy(obj_path, build_dir, build_dir_len);
    obj_path[build_dir_len] = '/';
    memcpy(obj_path + build_dir_len + 1, "tia.obj", 7);
    obj_path[obj_path_len] = '\0';

    if (LLVMTargetMachineEmitToFile(targetMachine, context.llvm_info.module, obj_path, LLVMObjectFile, &error) != 0) {
        red_printf("Failed to emit object file: %s\n", error);
        LLVMDisposeMessage(error);
        return NULL;
    } else {
        green_printf("Successfully compiled Tia project to: %s\n", obj_path);
    }

    u64 exe_path_len = build_dir_len + 8;
    char* exe_path = alloc(exe_path_len + 1);
    memcpy(exe_path, build_dir, build_dir_len);
    exe_path[build_dir_len] = '/';
    memcpy(exe_path + build_dir_len + 1, "tia.exe", 7);
    exe_path[exe_path_len] = '\0';

    bool res = link_obj_to_exe(obj_path, exe_path);
    if (!res) {
        red_printf("Failed to link obj: %s to exe: %s\n", obj_path, exe_path);
    } else {
        green_printf("Successfully linked obj: %s to exe: %s\n", obj_path, exe_path);
    }

    LLVMDisposeBuilder(context.llvm_info.builder);
    LLVMDisposeModule(context.llvm_info.module);
    LLVMContextDispose(context.llvm_info.llvm_context);

    return exe_path;
}
