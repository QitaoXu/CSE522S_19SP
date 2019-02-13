#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct node {
    int num;
    char *line;
    struct node* left;
    struct node* right;
    int height;
}

void dispose(node *root);
node* find( int e, node *t);
