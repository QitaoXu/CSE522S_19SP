#include <stdio.h>
#include <stdlib.h>
#include "tree.h"

int max(int a, int b) {
    return (a > b) ? a : b;
}

int height( struct Node * N) {
    if (N == NULL) return 0;

    return N->height;
}

struct Node * newNode( int key, char * line ) {
    struct Node * node = (struct Node *)malloc(sizeof(struct Node));
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    node->line = line;
    node->height = 1;
    return node;
}

struct Node *rightRotate( struct Node * y) {
    struct Node * x = y->left;
    struct Node * T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    return x;
}

struct Node *leftRotate( struct Node * x) {
    struct Node * y = x->right;
    struct Node * T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y;
}

int getBalance( Node * N ) {
    if (N == NULL) return 0;
    return height(N->left) - height(N->right);
}

struct Node * insert( struct Node * node, int key, char * line) {
    if (node == NULL) return newNode(key, line);

    if (key < node->key)
        node->left = insert(node->left, key, line);
    else if (key > node->key)
        node->right = insert(node->right, key, line);
    node->height = 1 + max(height(node->left), height(node->right));

    int balance = getBalance(node);

    if (balance > 1 && key < node->left->key) { // left left case
        return rightRotate(node);
    }

    if (balance < -1 && key > node->right->key ) { // right right case
        return leftRotate(node);
    }

    if (balance > 1 && key > node->left->key ) { // left right case
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }

    if (balance > -1 && key < node->right->key ) { // right left case
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

void preOrder(struct Node * root) {
    if (root != NULL) {
        printf("%d:%s\n", root->key, root->line);
        preOrder(root->left);
        preOrder(root->right);
    }
}

