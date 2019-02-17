#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Node
{
    int key;
    char* line;
    struct Node *left;
    struct Node *right;
    int height;
};

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
    node->line = NULL;
    if (line == NULL) {
        printf("Error: line is NULL\n");
        exit(-1);
    }
    if (strlen(line) < 1) {
        printf("Error: strlen(line): %lu\n", strlen(line));
        exit(-1);
    } 
    if (strlen(line) == 1) {
        //printf("NewNode: line: %ld\n", strlen(line));
        node->line = (char *)malloc(sizeof(char) * 2);
        if (node->line == NULL) {
            printf("malloc failed!\n");
            exit(-1);
        }
        (node->line)[0] = '\n';
        (node->line)[1] = '\0';
        
    }
    if (strlen(line) > 1) {
        //printf("NewNode: line: %s\n", line);
        node->line = (char *)malloc(sizeof(char) * strlen(line));
        if (node->line == NULL) {
            printf("Error: malloc() failed!\n");
            exit(-1);
        }

        strncpy(node->line, line, sizeof(char) * strlen(line));
        
    }
    
    node->height = 1;
    //printf("node->line: %s\n",node->line);
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

int getBalance( struct Node * N ) {
    if (N == NULL) return 0;
    return height(N->left) - height(N->right);
}

struct Node * insert( struct Node * node, int key, char * line) {
    struct Node * return_Node = NULL;
    if (node == NULL) {
        printf("key = %d\n\n\n\n", key);
        // return newNode(key, line);
        return_Node = newNode(key, line);
        if (return_Node == NULL) {
            printf("NULL\n");
            exit(-1);
        }
        return return_Node;
    }

    if (key < node->key) {
        printf("key = %d, node->key = %d\n\n", key, node->key);
        // node->left = insert(node->left, key, line);
        return_Node = insert(node->left, key, line);
        if (return_Node == NULL) {
            printf("NULL\n");
            exit(-1);
        }
        node->left = return_Node;
    }
    else if (key > node->key){
        printf("key = %d, node->key = %d\n\n", key, node->key);
        // node->right = insert(node->right, key, line);
        return_Node = insert(node->right, key, line);
        if (return_Node == NULL) {
            printf("NULL\n");
            exit(-1);
        }
        node->right = return_Node;
    }
    node->height = 1 + max(height(node->left), height(node->right));

    int balance = getBalance(node);

    if (balance > 1 && key < node->left->key) { // left left case
        printf("left-left case:\n");
        printf("****************\n\n\n");
        return rightRotate(node);
    }

    if (balance < -1 && key > node->right->key ) { // right right case
        printf("right-right case:\n");
        printf("****************\n\n\n");
        return leftRotate(node);
    }

    if (balance > 1 && key > node->left->key ) { // left right case
        printf("left-right case:\n");
        printf("****************\n\n\n");
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }

    if (balance > -1 && key < node->right->key ) { // right left case
        printf("right-left case:\n");
        printf("****************\n\n\n");
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

void inOrder(struct Node * root) {
    if (root != NULL) {
        inOrder(root->left);
        printf("%d:%s\n", root->key, root->line);
        inOrder(root->right);
    }
}

void destroy(struct Node * root) {
    if (root != NULL) {
        destroy(root->left);
        destroy(root->right);
        free(root->line);
        free(root);
    }
}

