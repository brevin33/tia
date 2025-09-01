#include "tia.h"

int main() {
    Arena arena = arena_create_root(1);
    Folder* folder = create_tia_project("C:/Users/brevi/dev/Tia/scratch", &arena);
    debugbreak();
    return 0;
}
