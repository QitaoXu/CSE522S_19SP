#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
#include <string.h>
int main() {
    struct Node * root = NULL;

    root = insert(root, 10, "10");
    root = insert(root, 20, "20");
    root = insert(root, 30, "30");
    root = insert(root, 40, "40");
    root = insert(root, 50, "50");
    root = insert(root, 60, " \n");
    printf("char len, %lu\n", strlen(" \n"));
    // preOrder(root);
    inOrder(root);

    destroy(root);

    return 0;
}