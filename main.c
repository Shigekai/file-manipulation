#include <stdio.h>

#include "imgdb.h"
#include "store.h"
#include "menu.h"

int main(void) {
    ensure_bin_folder();
    menu();
    return 0;
}