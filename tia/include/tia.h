#include "tia/basic.h"
#include "tia/arena.h"
#include "tia/lists.h"
#include "tia/token.h"
#include "tia/folder.h"
#include "tia/file.h"
#include "tia/logging.h"
#include "tia/ast.h"
#include "tia/type.h"
#include "tia/function.h"
#include "tia/statement.h"
#include "tia/expression.h"
#include "tia/scope.h"

typedef struct {
    Arena* arena;
    Folder_Pointer_List folders;
    File_Pointer_List files;
    Type_Base_Pointer_List types;
    Function_Pointer_List functions;
} Context;

extern Context context;

Folder* create_tia_project(char* path, Arena* arena);
