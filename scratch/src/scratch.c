#include "tia.h"

int main() {
    Arena arena = arena_create_root(1);
    Folder* folder = create_tia_project("C:/Users/brevi/dev/Tia/scratch", &arena);
    char* exe_path = compile_tia_project(folder);
    if (exe_path == NULL) {
        debugbreak();
        return 1;
    }
    printf("\nRunning exe: %s\n", exe_path);
    int res = system(exe_path);
    printf("\nDone returned: %d\n", res);
    debugbreak();
    return 0;
}
