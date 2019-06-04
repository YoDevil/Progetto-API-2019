#define ID_LEN 64
#define HASH_TABLE_SIZE 2048

typedef struct {
    char id[ID_LEN+1];
} entity_t;

typedef struct ht_entry_s {
    void* object;
    struct ht_entry_s* next;
} ht_entry_t; // Chaining

typedef struct {
    ht_entry_t* arr[HASH_TABLE_SIZE];
} ht_t;

typedef struct bst_node_s {
    void* object;
    struct bst_node_s* parent;
    struct bst_node_s* left;
    struct bst_node_s* right;
} bst_node_t;

typedef struct {
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

// http://www.cs.ecu.edu/karl/3300/spr14/Notes/DataStructure/hashtable.html
int strhash(const char* str)
{
    const char* p;
    int         g;
    int         h = 0;

    for(p = str; *p != '\0'; p++){
        h = (h << 4) + (int)(*p);
        g = h & 0xF0000000L;
        if(g != 0)
            h = h ^ (g >> 24);
        h = h & ~g;
    }
    return h%HASH_TABLE_SIZE;
}

ht_t* ht_create(){
    ht_t* table = calloc(1, sizeof(ht_t));
    return table;
}

void ht_free_row(ht_t* ht, char* key){
    int idx = strhash(key);
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
}

listht_t* listht_create(){
    listht_t* list = malloc(sizeof(listht_t));
    list->head = NULL;
    list->ht = ht_create();
    return list;
}

bstht_t* bstht_create(){
    bstht_t* tree = malloc(sizeof(bstht_t));
    tree->root = NULL;
    tree->ht = ht_create();
    return tree;
}

bst_node_t* bst_get_min(bst_node_t* x){
    if(x == NULL)
        return NULL;

    while(x->left != NULL)
        x = x->left;
    return x;
}

bst_node_t* bst_get_max(bst_node_t* x){
    if(x == NULL)
        return NULL;

    while(x->right != NULL)
        x = x->right;
    return x;
}

bst_node_t* bst_get_successor(bst_node_t* x){
    if(x == NULL)
        return NULL;

    bst_node_t* y;

    if(x->right != NULL)
        return bst_get_min(x->right);

    y = x->parent;
    while(y != NULL && x == y->right){
        x = y;
        y = y->parent;
    }
    return y;
}

bst_node_t* bst_get_predecessor(bst_node_t* x){
    if(x == NULL)
        return NULL;

    bst_node_t* y;

    if(x->left != NULL)
        return bst_get_max(x->left);

    y = x->parent;
    while(y != NULL && x == y->left){
        x = y;
        y = y->parent;
    }
    return y;
}

// get from the hash table
bst_node_t* bstht_get_node(bstht_t* tree, char* key, char* (*get_key)(bst_node_t*)){
    int idx = strhash(key);
    ht_entry_t* walk = tree->ht->arr[idx];
    while(walk){
        bst_node_t* bst_node = (bst_node_t*)walk->object;

        if(!strcmp(get_key(bst_node), key))
            return bst_node;
        walk = walk->next;
    }
    return NULL;
}

void bst_insert_node(bstht_t* tree, bst_node_t* z, int (*key_less_than)(bst_node_t*, bst_node_t*)){
    bst_node_t* y = NULL;
    bst_node_t* x = tree->root;

    while(x != NULL){
        y = x;
        if(key_less_than(z, x))
            x = x->left;
        else
            x = x->right;
    }
    z->parent = y;
    if(y == NULL)
        tree->root = z;
    else if(key_less_than(z, y))
        y->left = z;
    else y->right = z;
}

bst_node_t* bst_remove_node(bstht_t* tree, bst_node_t* z){
    bst_node_t *x, *y;

    if(z->left == NULL || z->right == NULL)
        y = z;
    else
        y = bst_get_successor(z);

    if(y->left != NULL)
        x = y->left;
    else
        x = y->right;

    if(x != NULL)
        x->parent = y->parent;

    if(y->parent == NULL)
        tree->root = x;
    else if(y == y->parent->left)
        y->parent->left = x;
    else
        y->parent->right = x;

    if(y != z)
        z->object = y->object;

    return y;
}

void bstht_free_node(bstht_t* tree, bst_node_t* bst_node, char* (*get_key)(bst_node_t*)){
    // Save the key now cause the bst_node->object may change after bst_remove_node
    char* key = get_key(bst_node);

    // Remove from tree
    bst_node_t* deleted = bst_remove_node(tree, bst_node);

    // Remove from ht
    connections_t* connections;
    int idx;
    ht_entry_t* walk;

    idx = strhash(key);

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
        connections = (connections_t*)deleted->object;
        idx = strhash(connections->me->id);

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
    bst_node_t* bst_node = calloc(1, sizeof(bst_node_t));
    bst_node->object = object;

    // BST insertion
    bst_insert_node(tree, bst_node, key_less_than);

    // HT insertion
    int idx = strhash(get_key(bst_node));
    ht_entry_t* ht_entry = malloc(sizeof(ht_entry_t));
    ht_entry->object = (void*)bst_node;
    ht_entry->next = tree->ht->arr[idx];
    tree->ht->arr[idx] = ht_entry;

    return bst_node;
}

void bstht_update_node(bstht_t* tree, bst_node_t* bst_node, char* (*get_key)(bst_node_t*), int (*key_less_than)(bst_node_t*, bst_node_t*)){
    void* object = bst_node->object;
    bstht_free_node(tree, bst_node, get_key);  // This will not free the object
    bstht_add_unique(tree, object, get_key, key_less_than);
}
