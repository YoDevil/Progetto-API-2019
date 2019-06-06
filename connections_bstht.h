// Key for the connections tree is the entity's receiving connection count
int _conn_key(bst_node_t* bst_node){
    return ((connections_t*)bst_node->object)->receiving_count;
}

// Secondary key, if the first is equal, is the entity name
// We sort in inverse alphabetical order
char* _conn_key_2(bst_node_t* bst_node){
    return ((connections_t*)bst_node->object)->me->id;
}

int _conn_key_less_than(bst_node_t* a, bst_node_t* b){
    return _conn_key(a) < _conn_key(b) ||
            (_conn_key(a) == _conn_key(b) && strcmp(_conn_key_2(a), _conn_key_2(b)) > 0);
}

bst_node_t* bstht_add_connections_unique(bstht_t* tree, connections_t* connections){
    return bstht_add_unique(tree, (void*)connections, _conn_key_2, _conn_key_less_than);
}

bst_node_t* bstht_get_connections_node(bstht_t* tree, char* key){
    return bstht_get_node(tree, key, _conn_key_2);
}

void bstht_free_connections_node(bstht_t* tree, bst_node_t* bst_node, bst_node_t** update_me){
    connections_t* connections = (connections_t*)bst_node->object;
    bstht_free_node(tree, bst_node, update_me, _conn_key_2);

    // Free the node->object
    free_entities_listht(connections->giving);
    free_entities_listht(connections->receiving);
    free(connections);
}

void bstht_update_connections_node(bstht_t* tree, bst_node_t* bst_node, bst_node_t** update_me){
    bstht_update_node(tree, bst_node, update_me, _conn_key_2, _conn_key_less_than);
}

void print_connections_tree(bst_node_t* x){
    if(x != NULL){
        print_connections_tree(x->right);
        printf("%s: %d\n", ((connections_t*)x->object)->me->id, ((connections_t*)x->object)->receiving_count);
        print_connections_tree(x->left);
    }
}
