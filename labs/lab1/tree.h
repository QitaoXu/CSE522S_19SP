#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

struct Node
{
    int key;
    char* line;
    struct Node *left;
    struct Node *right;
    int height;
};

int 
max( int a, int b );

int
height( struct Node *N );

struct Node*
newNode( int key );

struct Node*
rightrotate( struct Node *y );

struct Node*
leftRotate( struct Node *x );

int
getBalance( struct Node *N);

struct Node*
insert( struct Node * node, int key, char *line);

void 
preOrder( struct Node *root );


