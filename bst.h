#define BLACK 0
#define RED 1

void rb_insert_fixup(bst_t*, bst_node_t*);
void rb_delete_fixup(bst_t* tree, bst_node_t* x);

bst_node_t* NIL = NULL;

bst_t* bst_create(){
    // global NIL node initialization
    if(NIL == NULL){
        NIL = malloc(sizeof(bst_node_t));
        NIL->color = BLACK;
        NIL->parent = NIL;
        NIL->left = NIL;
        NIL->right = NIL;
        NIL->key = NULL;
    }

    bst_t* tree = malloc(sizeof(bst_t));
    tree->root = NIL;
    return tree;
}

bst_node_t* bst_alloc_node(char* key){
    bst_node_t* node = malloc(sizeof(bst_node_t));
    node->key = key;
    node->left = NIL;
    node->right = NIL;
    node->parent = NIL;
    return node;
}

bst_node_t* bst_get_min(bst_node_t* x){
    if(x == NIL)
        return NIL;

    while(x->left != NIL)
        x = x->left;
    return x;
}

bst_node_t* bst_get_max(bst_node_t* x){
    if(x == NIL)
        return NIL;

    while(x->right != NIL)
        x = x->right;
    return x;
}

bst_node_t* bst_get_successor(bst_node_t* x){
    if(x == NIL)
        return NIL;

    bst_node_t* y;

    if(x->right != NIL)
        return bst_get_min(x->right);

    y = x->parent;
    while(y != NIL && x == y->right){
        x = y;
        y = y->parent;
    }
    return y;
}

bst_node_t* bst_get_predecessor(bst_node_t* x){
    if(x == NIL)
        return NIL;

    bst_node_t* y;

    if(x->left != NIL)
        return bst_get_max(x->left);

    y = x->parent;
    while(y != NIL && x == y->left){
        x = y;
        y = y->parent;
    }
    return y;
}

bst_node_t* bst_search_recursive(bst_node_t* x, char* key){
    if(x == NIL)
        return x;

    int cmp = strcmp(key, x->key);
    if(cmp == 0)
        return x;
    else if(cmp < 0)
        return bst_search_recursive(x->left, key);
    else
        return bst_search_recursive(x->right, key);
}

bst_node_t* bst_get(bst_t* tree, char* key){
    return bst_search_recursive(tree->root, key);
}

void bst_insert(bst_t* tree, bst_node_t* z){
    bst_node_t* y = NIL;
    bst_node_t* x = tree->root;

    while(x != NIL){
        y = x;
        if(strcmp(z->key, x->key) < 0)
            x = x->left;
        else
            x = x->right;
    }
    z->parent = y;
    if(y == NIL)
        tree->root = z;
    else if(strcmp(z->key, y->key) < 0)
        y->left = z;
    else y->right = z;
    
    z->color = RED;
    rb_insert_fixup(tree, z);
}

int bst_get_or_alloc_and_insert(bst_node_t** node, bst_t* tree, char* key){
    bst_node_t* y = NIL;
    bst_node_t* x = tree->root;

    while(x != NIL){
        y = x;
        int cmp = strcmp(key, x->key);
        if(cmp == 0){
            *node = y;
            return 0;
        }
        else if(cmp < 0)
            x = x->left;
        else
            x = x->right;
    }

    bst_node_t* z = bst_alloc_node(key);
    z->parent = y;
    if(y == NIL)
        tree->root = z;
    else if(strcmp(z->key, y->key) < 0)
        y->left = z;
    else y->right = z;

    z->color = RED;
    rb_insert_fixup(tree, z);
    *node = z;
    return 1;
}

bst_node_t* bst_remove(bst_t* tree, bst_node_t* z){
    bst_node_t *x, *y;

    if(z->left == NIL || z->right == NIL)
        y = z;
    else
        y = bst_get_successor(z);

    if(y->left != NIL)
        x = y->left;
    else
        x = y->right;

    x->parent = y->parent;

    if(y->parent == NIL)
        tree->root = x;
    else if(y == y->parent->left)
        y->parent->left = x;
    else
        y->parent->right = x;

    if(y != z){
        z->key = y->key;
        z->object = y->object;
    }

    if(y->color == BLACK)
        rb_delete_fixup(tree, x);
    return y;
}

// ================= RB FIXUPS ================= //
void bst_left_rotate(bst_t* tree, bst_node_t* x){
    bst_node_t* y = x->right;

    x->right = y->left;

    if(y->left != NIL)
        y->left->parent = x;

    y->parent = x->parent;

    if(x->parent == NIL)
        tree->root = y;
    else if(x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;

    y->left = x;
    x->parent = y;
}

void bst_right_rotate(bst_t* tree, bst_node_t* x){
    bst_node_t* y = x->left;

    x->left = y->right;

    if (y->right != NIL)
        y->right->parent = x;

    y->parent = x->parent;

    if (x->parent == NIL)
        tree->root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;

    y->right = x;
    x->parent = y;
}

void rb_insert_fixup(bst_t* tree, bst_node_t* z){
    bst_node_t *x, *y;

    if(z == tree->root)
        tree->root->color = BLACK;
    else {
        x = z->parent;
        if(x->color == RED){
            if(x == x->parent->left){
                y = x->parent->right;
                if(y->color == RED){
                    x->color = BLACK;
                    y->color = BLACK;
                    x->parent->color = RED;
                    rb_insert_fixup(tree, x->parent);
                } else {
                    if(z == x->right){
                        z = x;
                        bst_left_rotate(tree, z);
                        x = z->parent;
                    }
                    x->color = BLACK;
                    x->parent->color = RED;
                    bst_right_rotate(tree, x->parent);
                }
            } else { // x == x->parent->right
                y = x->parent->left;
                if(y->color == RED){
                    x->color = BLACK;
                    y->color = BLACK;
                    x->parent->color = RED;
                    rb_insert_fixup(tree, x->parent);
                } else {
                    if(z == x->left){
                        z = x;
                        bst_right_rotate(tree, z);
                        x = z->parent;
                    }
                    x->color = BLACK;
                    x->parent->color = RED;
                    bst_left_rotate(tree, x->parent);
                }
            }
        }
    }
}

void rb_delete_fixup(bst_t* tree, bst_node_t* x){
    bst_node_t* w;

    if(x->color == RED || x->parent == NULL){
        x->color = BLACK;
    } else if(x == x->parent->left){
        w = x->parent->right;
        if(w->color == RED){
            w->color = BLACK;
            x->parent->color = RED;
            bst_left_rotate(tree, x->parent);
            w = x->parent->right;
        }
        if(w->left->color == BLACK && w->right->color == BLACK){
            w->color = RED;
            rb_delete_fixup(tree, x->parent);
        } else {
            if(w->right->color == BLACK){
                w->left->color = BLACK;
                w->color = RED;
                bst_right_rotate(tree, w);
                w = x->parent->right;
            }
            w->color = x->parent->color;
            x->parent->color = BLACK;
            w->right->color = BLACK;
            bst_left_rotate(tree, x->parent);
        }
    } else {
        w = x->parent->left;
        if(w->color == RED){
            w->color = BLACK;
            x->parent->color = RED;
            bst_right_rotate(tree, x->parent);
            w = x->parent->left;
        }
        if(w->right->color == BLACK && w->left->color == BLACK){
            w->color = RED;
            rb_delete_fixup(tree, x->parent);
        } else {
            if(w->left->color == BLACK){
                w->right->color = BLACK;
                w->color = RED;
                bst_left_rotate(tree, w);
                w = x->parent->left;
            }
            w->color = x->parent->color;
            x->parent->color = BLACK;
            w->left->color = BLACK;
            bst_right_rotate(tree, x->parent);
        }
    }
}
