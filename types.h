#define ID_LEN 32
#define HT_SIZE 128
#define MAX_CHAMPIONS 512

typedef struct ht_entry_s {
    char key[ID_LEN+1];
    struct ht_entry_s* next;
} ht_entry_t;

typedef ht_entry_t* ht_t[HT_SIZE];

typedef struct bst_node_s {
    char* key;
    void* object;
    struct bst_node_s* parent;
    struct bst_node_s* left;
    struct bst_node_s* right;
    char color;
} bst_node_t;

typedef struct {
    bst_node_t* NIL;
    bst_node_t* root;
} bst_t;

typedef struct {
    bst_node_t* champions[MAX_CHAMPIONS];
    int len;
    int max;
    int dirty;
} record_t;

typedef struct {
    record_t record;
    bst_t* connections_tree;
} relation_t;

typedef struct {
    bst_t* giving;
    bst_t* receiving;
    int receiving_count;
} connections_t;
