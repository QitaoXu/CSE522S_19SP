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
    char * test = (char *)malloc(sizeof(char) * 3);
    test[0] = '\n';
    test[1] = '\n';
    test[2] = '\0';
    root = insert(root, 70, test);
    // char * u = NULL;
    // u = NULL;
    // preOrder(root);
    inOrder(root);

    destroy(root);

    return 0;
}