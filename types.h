#define ID_LEN 32
#define BIG_HASH_TABLE_SIZE 128
#define SMALL_HASH_TABLE_SIZE 32

#define BLACK 0
#define RED 1

typedef struct {
    char id[ID_LEN+1];
} entity_t;

typedef struct ht_entry_s {
    void* object;
    struct ht_entry_s* next;
} ht_entry_t; // Chaining

typedef struct {
    ht_entry_t** arr;
    size_t size;
} ht_t;

typedef struct bst_node_s {
    void* object;
    int color;
    struct bst_node_s* parent;
    struct bst_node_s* left;
    struct bst_node_s* right;
} bst_node_t;

typedef struct {
    bst_node_t* NIL;
    bst_node_t* root;
    ht_t* ht;
} bstht_t;

typedef struct {
    char id[ID_LEN+1];
    bstht_t* connections;
} relation_t;

typedef struct list_item_s {
    void* object;
    struct list_item_s* prev;
    struct list_item_s* next;
} list_item_t;

typedef struct {
    list_item_t* head;
    ht_t* ht;
} listht_t;

typedef struct {
    entity_t* me;
    listht_t* giving;       // List of the relations from me
    listht_t* receiving;    // List of the relations to me
    int receiving_count;    // How many relations to me, key for the bst
} connections_t;

void rb_insert_fixup(bstht_t*, bst_node_t*);
void rb_delete_fixup(bstht_t*, bst_node_t*);

// http://www.cs.ecu.edu/karl/3300/spr14/Notes/DataStructure/hashtable.html
int strhash(ht_t* table, const char* str)
{
    size_t i, sum;
    sum = 0;
    for(i = 0; str[i] != '\0'; i++)
        sum += (str[i] - '-' + 1) * (i + 1);

    return sum % (table->size);
}

ht_t* ht_create(size_t size){
    ht_t* table = malloc(sizeof(ht_t));
    table->size = size;
    table->arr = calloc(size, sizeof(ht_entry_t*));
    return table;
}

void ht_free_row(ht_t* ht, char* key){
    int idx = strhash(ht, key);
    ht_entry_t* walk = ht->arr[idx];
    ht_entry_t* tmp;
    while(walk){
        tmp = walk->next;
        free(walk);
        walk = tmp;
    }
    ht->arr[idx] = NULL;
}

// Somebody needs to call ht_free_row before me
void free_ht(ht_t* ht){
    free(ht->arr);
    free(ht);
}

listht_t* listht_create(size_t size){
    listht_t* list = malloc(sizeof(listht_t));
    list->head = NULL;
    list->ht = ht_create(size);
    return list;
}

bstht_t* bstht_create(size_t size){
    bstht_t* tree = malloc(sizeof(bstht_t));
    
    bst_node_t* nil = calloc(1, sizeof(bst_node_t));
    nil->color = BLACK;
    nil->parent = nil;
    nil->left = nil;
    nil->right = nil;
    
    tree->NIL = nil;
    tree->root = tree->NIL;
    tree->ht = ht_create(size);
    return tree;
}

void free_bstht(bstht_t* tree){
    // Assuming all nodes are freed
    free_ht(tree->ht);
    free(tree->NIL);
    free(tree);
}

bst_node_t* bst_get_min(bstht_t* tree, bst_node_t* x){
    if(x == tree->NIL)
        return tree->NIL;

    while(x->left != tree->NIL)
        x = x->left;
    return x;
}

bst_node_t* bst_get_max(bstht_t* tree, bst_node_t* x){
    if(x == tree->NIL)
        return tree->NIL;

    while(x->right != tree->NIL)
        x = x->right;
    return x;
}

bst_node_t* bst_get_successor(bstht_t* tree, bst_node_t* x){
    if(x == tree->NIL)
        return tree->NIL;

    bst_node_t* y;

    if(x->right != tree->NIL)
        return bst_get_min(tree, x->right);

    y = x->parent;
    while(y != tree->NIL && x == y->right){
        x = y;
        y = y->parent;
    }
    return y;
}

bst_node_t* bst_get_predecessor(bstht_t* tree, bst_node_t* x){
    if(x == tree->NIL)
        return tree->NIL;

    bst_node_t* y;

    if(x->left != tree->NIL)
        return bst_get_max(tree, x->left);

    y = x->parent;
    while(y != tree->NIL && x == y->left){
        x = y;
        y = y->parent;
    }
    return y;
}

// get from the hash table
bst_node_t* bstht_get_node(bstht_t* tree, char* key, char* (*get_key)(bst_node_t*)){
    int idx = strhash(tree->ht, key);
    ht_entry_t* walk = tree->ht->arr[idx];
    while(walk){
        bst_node_t* bst_node = (bst_node_t*)walk->object;

        if(!strcmp(get_key(bst_node), key))
            return bst_node;
        walk = walk->next;
    }
    return tree->NIL;
}

void bst_insert_node(bstht_t* tree, bst_node_t* z, int (*key_less_than)(bst_node_t*, bst_node_t*)){
    bst_node_t* y = tree->NIL;
    bst_node_t* x = tree->root;

    while(x != tree->NIL){
        y = x;
        if(key_less_than(z, x))
            x = x->left;
        else
            x = x->right;
    }
    z->parent = y;
    if(y == tree->NIL)
        tree->root = z;
    else if(key_less_than(z, y))
        y->left = z;
    else y->right = z;
    
    z->color = RED;
    rb_insert_fixup(tree, z);
}

bst_node_t* bst_remove_node(bstht_t* tree, bst_node_t* z, bst_node_t** update_me){
    bst_node_t *x, *y;
    if(update_me && *update_me == z)
        *update_me = tree->NIL;

    if(z->left == tree->NIL || z->right == tree->NIL)
        y = z;
    else
        y = bst_get_successor(tree, z);

    if(y->left != tree->NIL)
        x = y->left;
    else
        x = y->right;

    x->parent = y->parent;

    if(y->parent == tree->NIL)
        tree->root = x;
    else if(y == y->parent->left)
        y->parent->left = x;
    else
        y->parent->right = x;

    if(y != z){
        z->object = y->object;

        if(update_me && *update_me == y)
                *update_me = z;
    }

    if(y->color == BLACK)
        rb_delete_fixup(tree, x);
    return y;
}

void bstht_free_node(bstht_t* tree, bst_node_t* bst_node, bst_node_t** update_me, char* (*get_key)(bst_node_t*)){
    // Save the key now cause the bst_node->object may change after bst_remove_node
    char* key = get_key(bst_node);

    // Remove from tree
    bst_node_t* deleted = bst_remove_node(tree, bst_node, update_me);

    // Remove from ht
    int idx;
    ht_entry_t* walk;

    idx = strhash(tree->ht, key);

    walk = tree->ht->arr[idx];
    if(walk->object == bst_node){
        tree->ht->arr[idx] = walk->next;
        free(walk);
        if(deleted == bst_node)
            free(bst_node);
    } else {
        while(walk->next){
            if(walk->next->object == bst_node) {
                ht_entry_t* tbd = walk->next;
                walk->next = walk->next->next;
                free(tbd);
                if(deleted == bst_node)
                    free(bst_node);
                break;
            }
            walk = walk->next;
        }
    }

    if(deleted != bst_node){
        idx = strhash(tree->ht, get_key(deleted));

        walk = tree->ht->arr[idx];
        while(walk){
            if(walk->object == deleted) {
                walk->object = (void*)bst_node;
                free(deleted);
                break;
            }
            walk = walk->next;
        }
    }
}

bst_node_t* bstht_add_unique(bstht_t* tree, void* object, char* (*get_key)(bst_node_t*), int (*key_less_than)(bst_node_t*, bst_node_t*)){
    bst_node_t* bst_node = malloc(sizeof(bst_node_t));
    bst_node->object = object;
    bst_node->parent = tree->NIL;
    bst_node->left = tree->NIL;
    bst_node->right = tree->NIL;

    // BST insertion
    bst_insert_node(tree, bst_node, key_less_than);

    // HT insertion
    int idx = strhash(tree->ht, get_key(bst_node));
    ht_entry_t* ht_entry = malloc(sizeof(ht_entry_t));
    ht_entry->object = (void*)bst_node;
    ht_entry->next = tree->ht->arr[idx];
    tree->ht->arr[idx] = ht_entry;

    return bst_node;
}

void bstht_update_node(bstht_t* tree, bst_node_t* bst_node, bst_node_t** update_me, char* (*get_key)(bst_node_t*), int (*key_less_than)(bst_node_t*, bst_node_t*)){
    int should_update = update_me ? *update_me == bst_node : 0;

    void* object = bst_node->object;
    bstht_free_node(tree, bst_node, update_me, get_key);  // This will not free the object
    bst_node_t* new_node = bstht_add_unique(tree, object, get_key, key_less_than);

    if(should_update)
        *update_me = new_node;
}
