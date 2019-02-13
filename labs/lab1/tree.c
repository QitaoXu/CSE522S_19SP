#include "tree.h"

void dispose(node *root) {

    if (root != null) {
        dispose(root->left);
        dispose(root->right);
        free(t);
    }
}