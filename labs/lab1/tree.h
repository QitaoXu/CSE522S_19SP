/*
 * Discliamer: We implemented AVL tree interface according to 
 * what we learnt on website: geeksforgeeks.
 * Here is link: https://www.geeksforgeeks.org/avl-tree-set-1-insertion/
 * We changed part of Node field and add functions including inOrder, 
 * postOrder, inOrderSend, inOrderWrite and destroy to finish 522S lab 1.
 *
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

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
    if (N == NULL) {
        return 0;
    }

    return N->height;
}

struct Node * newNode( int key, char * line ) {
    struct Node * node = (struct Node *)malloc(sizeof(struct Node));
    if (node == NULL) {
        printf("node malloc failed!\n");
        exit(-1);
    }
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    node->line = NULL;
    if (line == NULL) {
        printf("Error: line is NULL\n");
        exit(-1);
    }
    // node->line = line;
    
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
    if (node == NULL) {
        return newNode(key, line);
    }

    if (key < node->key) {
        // printf("key = %d, node->key = %d\n\n", key, node->key);
        node->left = insert(node->left, key, line);
    }
    else if (key > node->key){
        node->right = insert(node->right, key, line);
    }
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

    if (balance < -1 && key < node->right->key ) { // right left case
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

void preOrder(struct Node * root) {
    if (root != NULL) {
        if (root->key > 100) {
            printf("Strange key: %d\n", root->key);
        }
        printf("%d:%s\n", root->key, root->line);
        preOrder(root->left);
        preOrder(root->right);
    }
}

void inOrder(struct Node * root) {
    int i;
    if (root != NULL) {
        inOrder(root->left);
        if (root->key > 100) {
            printf("Strange key: %d\n", root->key);
        }
        if ( strlen(root->line) > 1 && (root->line)[strlen(root->line) - 1] != '\n') {
            // printf("Strange line: %s\n\n\n", root->line);
            for(i = (strlen(root->line) - 1); i > 0; i--) {
                if((root->line)[i] == '\n') {
                    (root->line)[i + 1] = '\0';
                    break;
                }
            }
        }

        if ( strlen(root->line) > 1 && (root->line)[strlen(root->line) - 2] == '\n' && (root->line)[strlen(root->line) - 1] == '\n') {
            (root->line)[strlen(root->line) - 1] = '\0';
        }

        printf("%d:\n%s", root->key, root->line);
        inOrder(root->right);
    }
}

void destroy(struct Node * root) {
    if (root != NULL) {
        destroy(root->left);
        destroy(root->right);
        // free(root->line);
        free(root);
    }
}

void inOrderSend(struct Node * root, int skt) {
    int ret_write, i;
    if (root != NULL) {
        inOrderSend(root->left, skt);
        if (root->key > 100) {
            printf("Strange key: %d\n", root->key);
        }
        
        if ( strlen(root->line) > 1 && (root->line)[strlen(root->line) - 1] != '\n') {
            // printf("Strange line: %s\n\n\n", root->line);
            for(i = (strlen(root->line) - 1); i > 0; i--) {
                if((root->line)[i] == '\n') {
                    (root->line)[i + 1] = '\0';
                    break;
                }
            }
        }
        
        // printf("%s", root->line);
        ret_write = write(skt, root->line, strlen(root->line));
        if (ret_write < 0) {
            printf("Error: write system call when sending failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }
        
        inOrderSend(root->right, skt);
    }
}

void inOrderWrite(struct Node * root, FILE *fp) {
    int ret_fprintf, i;
    if (root != NULL) {
        inOrderWrite(root->left, fp);
        if (root->key > 100) {
            printf("Strange key: %d\n", root->key);
        }
        
        if ( strlen(root->line) > 1 && (root->line)[strlen(root->line) - 1] != '\n') {
            // printf("Strange line: %s\n\n\n", root->line);
            for(i = (strlen(root->line) - 1); i > 0; i--) {
                if((root->line)[i] == '\n') {
                    (root->line)[i + 1] = '\0';
                    break;
                }
            }
        }

        if ( strlen(root->line) > 1 && (root->line)[strlen(root->line) - 2] == '\n' && (root->line)[strlen(root->line) - 1] == '\n') {
            (root->line)[strlen(root->line) - 1] = '\0';
        }
        
        ret_fprintf = fprintf(fp, "%s", root->line);
        if (ret_fprintf < 0) {
            printf("Error: fprintf function failed! Reason: %s\n", strerror(errno));
            exit(-1);
        }
        
        inOrderWrite(root->right, fp);
    }
}
