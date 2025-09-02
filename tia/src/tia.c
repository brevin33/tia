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
    return folder;
}

bool compile_tia_project(Folder* folder) {
    if (context.numberOfErrors > 0) {
        red_printf("can't compile project with errors\n");
        red_printf("number of errors: %u\n", context.numberOfErrors);
        return false;
    }
    LLVMInitializeAllAsmParsers();
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmPrinters();
    LLVMInitializeAllDisassemblers();
    char* triple = LLVMGetDefaultTargetTriple();

    context.llvm_info.llvm_context = LLVMContextCreate();
    context.llvm_info.current_block = NULL;
    context.llvm_info.module = LLVMModuleCreateWithName(folder->name);
    context.llvm_info.builder = LLVMCreateBuilder();

    char* error;
    LLVMTargetRef target;
    if (LLVMGetTargetFromTriple(triple, &target, &error) != 0) {
        fprintf(stderr, "Failed to get target: %s\n", error);
        LLVMDisposeMessage(error);
        exit(1);
    }

    LLVMTargetMachineRef targetMachine;
    targetMachine = LLVMCreateTargetMachine(target, triple, "", "", LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault);

    context.llvm_info.data_layout = LLVMCreateTargetDataLayout(targetMachine);

    // compile functions that need compiled
    for (u64 i = 0; i < context.functions.count; i++) {
        Function* function = function_pointer_list_get_function(&context.functions, i);
        // allways compile main
        if (strcmp(function->name, "main") == 0) {
            Type_Substitution_List substitutions = type_substitution_list_create(0);
            function_compile_llvm(function, &substitutions);
        }

        // other things that we might compile
    }

    char* ir = LLVMPrintModuleToString(context.llvm_info.module);
    printf("\n%s\n\n", ir);
    LLVMDisposeMessage(ir);

    if (LLVMVerifyModule(context.llvm_info.module, LLVMAbortProcessAction, &error) != 0) {
        red_printf("LLVMVerifyModule failed: %s\n", error);
        LLVMDisposeMessage(error);
        massert(false, "LLVMVerifyModule failed");
    } else {
        green_printf("successfully verified module\n");
    }

    LLVMDisposeBuilder(context.llvm_info.builder);
    LLVMDisposeModule(context.llvm_info.module);
    LLVMContextDispose(context.llvm_info.llvm_context);
    return true;
}
