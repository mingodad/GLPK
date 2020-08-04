//
// A splay tree is a self-balancing binary search tree with the
// additional property that recently accessed elements are quick
// to access again. It performs basic operations such as insertion,
// look-up and removal in O(log(n)) amortized time.
//
// Adapted from the V8 project. Copyright 2011 the V8 project
// authors. All rights reserved. Maintained by http://github.com/hij1nx
//

#include "splaytree.h"
#include "dmp.h"
#include "env.h"


typedef struct SplayTree_node_s SplayTree_node_t;

struct SplayTree_node_s
{
    SplayTree_node_t *left, *right;
    const void *key;
};

struct SplayTree_t
{
    SplayTree_node_t *root;
    SplayTree_CmpFunc cmp_func;
    void *info;
    DMP *pool;
    /* memory pool for allocating nodes */
    int count;
};

static SplayTree_node_t *SplayTree_newNode(SplayTree_t *t, const void *key);
static void  SplayTree_splay(SplayTree_t *t, const void *key);

int SplayTree_strcmp(void *info, const void *key1, const void *key2)
{     /* compare character string keys */
      xassert(info == info);
      return strcmp(key1, key2);
}

SplayTree_t *SplayTree_New(SplayTree_CmpFunc cmp, void *info) {
	SplayTree_t *tree = xmalloc(sizeof(SplayTree_t));
	memset(tree, 0, sizeof(SplayTree_t));
	tree->cmp_func = cmp;
	tree->info = info;
        tree->pool = dmp_create_pool();
	return tree;
}

void SplayTree_Free(SplayTree_t *t) {
    if(!t) return;
    dmp_delete_pool(t->pool);
    xfree(t);
}

int  SplayTree_isEmpty(SplayTree_t *t) {
    return t->root == NULL;
}

int  SplayTree_count(SplayTree_t *t) {
    return t->count;
}

static SplayTree_node_t *SplayTree_newNode(SplayTree_t *t, const void *key) {
	SplayTree_node_t *node = dmp_get_atom(t->pool, sizeof(SplayTree_node_t));
	memset(node, 0, sizeof(SplayTree_node_t));
	node->key = key;
	return node;
  }

  //
  // ### function insert(key, value)
  // #### @param key {number} key Key to insert into the tree.
  // #### @param value {*} value Value to insert into the tree.
  // Inserts a node into the tree with the specified key and value if
  // the tree does not already contain a node with the specified key. If
  // the value is inserted, it becomes the root of the tree.
  //
int  SplayTree_insert(SplayTree_t *t, const void *key) {
    if (SplayTree_isEmpty(t)) {
      t->root = SplayTree_newNode(t, key);
      t->count = 1;
      return 1;
    }
    // Splay on the key to move the last node on the search path for
    // the key to the root of the tree.
    SplayTree_splay(t, key);
    int rc_cmp = t->cmp_func(t->info, t->root->key, key);
    if (rc_cmp == 0) {
      return 0;
    }
    SplayTree_node_t *node = SplayTree_newNode(t, key);
    if (rc_cmp < 0) {
      node->left = t->root;
      node->right = t->root->right;
      t->root->right = NULL;
    } else {
      node->right = t->root;
      node->left = t->root->left;
      t->root->left = NULL;
    }
    t->root = node;
    ++t->count;
    return 1;
}

  //
  // ### function remove(key)
  // #### @param {number} key Key to find and remove from the tree.
  // #### @return {SplayTree.Node} The removed node.
  // Removes a node with the specified key from the tree if the tree
  // contains a node with this key. The removed node is returned. If the
  // key is not found, an exception is thrown.
  //
const void *SplayTree_remove(SplayTree_t *t, const void *key) {
    if (SplayTree_isEmpty(t)) return NULL;

    SplayTree_splay(t, key);
    int rc_cmp = t->cmp_func(t->info, t->root->key, key);
    if (rc_cmp) {
      return NULL;
    }
    SplayTree_node_t *root = t->root;
    const void *value = t->root->key;
    if (!t->root->left) {
      t->root = t->root->right;
    } else {
      SplayTree_node_t *right = t->root->right;
      t->root = t->root->left;
      //
      // Splay to make sure that the new root has an empty right child.
      //
      SplayTree_splay(t, key);
      //
      // Insert the original right child as the right child of the new
      // root.
      //
      t->root->right = right;
    }
    --t->count;
    dmp_free_atom(t->pool, root, sizeof(SplayTree_node_t));
    return value;
  }

  //
  // ### function find(key)
  // #### @param {number} key Key to find in the tree.
  // #### @return {SplayTree.Node} Node having the specified key.
  // Returns the node having the specified key or null if the
  // tree doesn't contain a node with the specified key.
  //
const void *SplayTree_find(SplayTree_t *t, const void *key) {
    if (SplayTree_isEmpty(t)) return NULL;

    SplayTree_splay(t, key);
    int rc_cmp = t->cmp_func(t->info, t->root->key, key);
    return rc_cmp ? NULL : t->root->key;
}

const void *SplayTree_peek(SplayTree_t *t) {
    if (SplayTree_isEmpty(t)) return NULL;
    return t->root->key;
}
  //
  // ### function(opt_startNode)
  // #### @return {SplayTree.Node} Node having the maximum key value.
  //
static SplayTree_node_t *SplayTree_findMax(SplayTree_t *t, SplayTree_node_t *opt_startNode) {
    if (SplayTree_isEmpty(t)) return NULL;
    SplayTree_node_t * current = opt_startNode ?  opt_startNode : t->root;
    while (current->right) {
      current = current->right;
    }
    return current;
}

  //
  // ### function findGreatestLessThan(key)
  // #### @return {SplayTree.Node} Node having the maximum key value that
  //     is less than the specified key value.
  //
const void *SplayTree_findGreatestLessThan(SplayTree_t *t, const void *key) {
    if (SplayTree_isEmpty(t)) return NULL;

    //
    // Splay on the key to move the node with the given key or the last
    // node on the search path to the top of the tree.
    //
    SplayTree_splay(t, key);
    int rc_cmp = t->cmp_func(t->info, t->root->key, key);
    //
    // Now the result is either the root node or the greatest node in
    // the left subtree.
    //
    //if (this.root.key < key) {
    if (rc_cmp < 0) {
      return t->root;
    } else if (t->root->left) {
      return SplayTree_findMax(t, t->root->left)->key;
    }
    return NULL;
}

  //
  // This is the simplified top-down splaying algorithm from:
  // * "Self-adjusting Binary Search Trees" by Sleator and Tarjan
  //
static void  SplayTree_splay(SplayTree_t *t, const void *key) {
    if (SplayTree_isEmpty(t)) return;

    SplayTree_node_t stub, *left, *right;
    left = right = &stub;
    stub.right = stub.left = NULL;

    SplayTree_node_t *root = t->root;

    while (1) {
      int rc_cmp = t->cmp_func(t->info, key, root->key);
      if (rc_cmp < 0) {

        if (!root->left) {
          break;
        }

        //if (key < root.left.key) {
        if (t->cmp_func(t->info, key, root->left->key) < 0) {
          // Rotate right.
          SplayTree_node_t *tmp = root->left;
          root->left = tmp->right;
          tmp->right = root;
          root = tmp;
          if (!root->left) {
            break;
          }
        }

        // Link right.
        right->left = root;
        right = root;
        root = root->left;

      } else if (rc_cmp > 0) {
        if (!root->right) {
          break;
        }
        if (t->cmp_func(t->info, key, root->right->key) > 0) {
          // Rotate left.
          SplayTree_node_t *tmp = root->right;
          root->right = tmp->left;
          tmp->left = root;
          root = tmp;
          if (!root->right) {
            break;
          }
        }
        // Link left.
        left->right = root;
        left = root;
        root = root->right;
      } else {
        break;
      }
    }

    // Assemble.
    left->right = root->left;
    right->left = root->right;
    root->left = stub.right;
    root->right = stub.left;
    t->root = root;
  };

  //
  // ### function traverse()
  // #### @param {function(SplayTree.Node)} f Visitor function.
  // #### @private
  // Performs an ordered traversal of the subtree starting at
  // this SplayTree.Node.
  //

void  SplayTree_traverse(SplayTree_t *t, int (*f)(SplayTree_t *t, SplayTree_node_t *n)) {
    SplayTree_node_t *current = t->root;
    while (current) {
      SplayTree_node_t *left = current->left;
      if (left) SplayTree_traverse(t, f);
      f(t, current);
      current = current->right;
    }
}

