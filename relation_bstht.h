// Key for the relations tree is their name(id).
// We sort by alphabetical order.
char* _rel_key(bst_node_t* entry){
    return ((relation_t*)entry->object)->id;
}

int _rel_key_less_than(bst_node_t* a, bst_node_t* b){
    return strcmp(_rel_key(a), _rel_key(b)) < 0;
}

void bstht_add_relation_unique(bstht_t* tree, relation_t* relation){
    bstht_add_unique(tree, (void*)relation, _rel_key, _rel_key_less_than);
}

bst_node_t* bstht_get_relation_node(bstht_t* tree, char* key){
    return bstht_get_node(tree, key, _rel_key);
}

void bstht_free_relation_node(bstht_t* tree, bst_node_t* bst_node, bst_node_t** update_me){
    relation_t* relation = (relation_t*)bst_node->object;
    bstht_free_node(tree, bst_node, update_me, _rel_key);

    // Free the node->object
    free_bstht(relation->connections);
    free(relation);
}