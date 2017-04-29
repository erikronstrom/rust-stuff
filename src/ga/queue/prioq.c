#include "ga/queue/prioq.h"

#include "ga/alloc.h"
#include <stdio.h>

typedef struct qnode qnode;

struct qnode {
    void*  value;
    qnode* left;
    qnode* right;
};

struct ga_prioq {
    qnode* root;          // Root of the binary tree (or head of the skew heap)
    ga_prioq_cmpfn cmpfn; // Compare function
    qnode* free_nodes;    // List of nodes for reuse, linked using their left member
};

// -----------------------------------------------------------------------------

inline static qnode* new_node(ga_prioq *queue, void *value, qnode *left, qnode *right)
{
    qnode *node;
    if (queue->free_nodes) {
        // Resue a free node
        // Since all struct members are set below, there is no risk of
        // old data still being around
        node = queue->free_nodes;
        queue->free_nodes = node->left;
    } else {
        node = ga_new(qnode);
    }
    node->value = value;
    node->left  = left;
    node->right = right;
    return node;
}

inline static void delete_node(ga_prioq *queue, qnode *node)
{
    node->value = NULL; // not needed, but as a precaution
    node->right = NULL;
    // Push the node to the free_nodes list, rather than freeing its memory
    node->left = queue->free_nodes;
    queue->free_nodes = node;
}

inline static void recursive_delete_node(qnode *node)
{
    if (!node) return;
    recursive_delete_node(node->left);
    recursive_delete_node(node->right);
    ga_free(node);
}

// -----------------------------------------------------------------------------

ga_prioq* ga_prioq_create(ga_prioq_cmpfn cmpfn)
{
    ga_prioq *queue = ga_newc(ga_prioq);
    queue->cmpfn = cmpfn;
    return queue;
}

void ga_prioq_destroy(ga_prioq *queue)
{
    recursive_delete_node(queue->root);
    recursive_delete_node(queue->free_nodes);
    ga_free(queue);
}

void ga_prioq_preallocate(ga_prioq *queue, unsigned int count)
{
    for (unsigned int i = 0; i < count; i++) {
        qnode *node = ga_newc(qnode);
        node->left = queue->free_nodes;
        queue->free_nodes = node;
    }
}

static inline qnode* recursive_merge(qnode *node1, qnode *node2, ga_prioq_cmpfn cmp);

static inline qnode* into(qnode *node1, qnode *node2, ga_prioq_cmpfn cmp)
{
    qnode *tmp   = node1->left;
    node1->left  = recursive_merge(node2, node1->right, cmp);
    node1->right = tmp;
    return node1;
}

static inline qnode* recursive_merge(qnode *node1, qnode *node2, ga_prioq_cmpfn cmp)
{
    if (!node1) {
        return node2;
    } else if (!node2) {
        return node1;
    } else {
        if (cmp(node1->value, node2->value) < 0) {
            return into(node1, node2, cmp);
        } else {
            return into(node2, node1, cmp);
        }
    }
}

void ga_prioq_push(ga_prioq *queue, void *value)
{
    queue->root = recursive_merge(queue->root, new_node(queue, value, NULL, NULL), queue->cmpfn);
}

void* ga_prioq_peek(const ga_prioq *queue)
{
    qnode *root = queue->root;
    return root ? root->value : NULL;
}

void* ga_prioq_pop(ga_prioq *queue)
{
    qnode *root = queue->root;

    if (!root) {
        return NULL;
    } else {
        void *value = root->value;
        queue->root = recursive_merge(root->left, root->right, queue->cmpfn);
        delete_node(queue, root);
        return value;
    }
}

