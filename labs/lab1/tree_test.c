#include <stdio.h>
#include <stdlib.h>
#include "tree.h"

int main() {
    struct Node * root = NULL;

    root = insert(root, 10, "10");
    root = insert(root, 20, "20");
    root = insert(root, 30, "30");
    root = insert(root, 40, "40");
    root = insert(root, 50, "50");
    root = insert(root, 60, "60");

    return 0;
}