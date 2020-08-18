#include "tree_node.h"
#include <stdlib.h>

tree_node_t *make_interior(op_type_t op, char *token, tree_node_t *left, tree_node_t *right) {
    interior_node_t *new_interior = (interior_node_t *)malloc(sizeof(interior_node_t));
    tree_node_t *new_tree_node = (tree_node_t *)malloc(sizeof(tree_node_t));

    if (new_interior == NULL || new_tree_node == NULL)
        return NULL;

    new_interior -> op = op;
    new_interior -> left = left;
    new_interior -> right = right;

    new_tree_node -> type = INTERIOR;
    new_tree_node -> token = token;
    new_tree_node -> node = new_interior;

    return new_tree_node;
}

tree_node_t *make_leaf(exp_type_t exp_type, char *token) {
    leaf_node_t *new_leaf = (leaf_node_t *)malloc(sizeof(leaf_node_t));
    tree_node_t *new_tree_node = (tree_node_t *)malloc(sizeof(tree_node_t));

    if (new_leaf == NULL || new_tree_node == NULL)
        return NULL;

    new_leaf -> exp_type = exp_type;

    new_tree_node -> type = LEAF;
    new_tree_node -> token = token;
    new_tree_node -> node = new_leaf;

    return new_tree_node;
}