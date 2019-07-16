void bst_left_rotate(bstht_t* tree, bst_node_t* x){
    bst_node_t* y = x->right;

    x->right = y->left;

    if(y->left != tree->NIL)
        y->left->parent = x;

    y->parent = x->parent;

    if(x->parent == tree->NIL)
        tree->root = y;
    else if(x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;

    y->left = x;
    x->parent = y;
}

void bst_right_rotate(bstht_t* tree, bst_node_t* x){
    bst_node_t* y = x->left;

    x->left = y->right;

    if (y->right != tree->NIL)
        y->right->parent = x;

    y->parent = x->parent;

    if (x->parent == tree->NIL)
        tree->root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;

    y->right = x;
    x->parent = y;
}

void rb_insert_fixup(bstht_t* tree, bst_node_t* z){
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

void rb_delete_fixup(bstht_t* tree, bst_node_t* x){
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