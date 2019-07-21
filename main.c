#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "ht.h"
#include "bst.h"

connections_t* alloc_connections();
relation_t* alloc_relation();
void insertion_sort(bst_node_t*[], int);
void add_entity(ht_t, char*);
void del_entity(ht_t, bst_t*, char*);
void add_connection(bst_t*, char*, char*, char*);
void del_connection(bst_t*, char*, char*, char*);
void report(bst_t*);

int main(){
    char cmd[ID_LEN+1];
    char arg1[ID_LEN+1];
    char arg2[ID_LEN+1];
    char arg3[ID_LEN+1];
    int ok;
           
    ht_t tracked = {0};
    bst_t* relations_tree = bst_create();

    while(1){
        ok = scanf("%s", cmd);

        if (ok && !strcmp(cmd, "addent")){
            ok = scanf(" \"%[^\"]\"", arg1);
            add_entity(tracked, arg1);

        } else if(ok && !strcmp(cmd, "delent")){
            ok = scanf(" \"%[^\"]\"", arg1);

            del_entity(tracked, relations_tree, arg1);

        } else if(ok && !strcmp(cmd, "addrel")){
            ok = scanf(" \"%[^\"]\" \"%[^\"]\" \"%[^\"]\"", arg1, arg2, arg3);

            if(ht_search(tracked, arg1) && ht_search(tracked, arg2))
                add_connection(relations_tree, arg3, arg1, arg2);

        } else if(ok && !strcmp(cmd, "delrel")){
            ok = scanf(" \"%[^\"]\" \"%[^\"]\" \"%[^\"]\"", arg1, arg2, arg3);
            if(ht_search(tracked, arg1) && ht_search(tracked, arg2))
                del_connection(relations_tree, arg3, arg1, arg2);

        } else if(ok && !strcmp(cmd, "report")){
            report(relations_tree);
        } else if(ok && !strcmp(cmd, "end")){
            fflush(stdout);
            break;
        }

        if(!ok)
            abort();
    }

    return 0;
}

void add_entity(ht_t tracked, char* id){
    // TODO: this can be optimized: combine search and insert
    if(ht_search(tracked, id) == 0)
        ht_insert(tracked, id);
}

void add_connection(bst_t* relations_tree, char* rel_id, char* from, char* to){
    // Get relation
    bst_node_t* relation_node;
    int new = bst_get_or_alloc_and_insert(&relation_node, relations_tree, rel_id);
    if(new)
        relation_node->object = alloc_relation();

    // Get sender and recipient
    relation_t* relation = relation_node->object;
    bst_node_t* sender;
    new = bst_get_or_alloc_and_insert(&sender, relation->connections_tree, from);
    if(new)
        sender->object = alloc_connections();
    bst_node_t* recipient;
    new = bst_get_or_alloc_and_insert(&recipient, relation->connections_tree, to);
    if(new)
        recipient->object = alloc_connections();
    
    bst_node_t* other;
    new = bst_get_or_alloc_and_insert(&other, ((connections_t*)sender->object)->giving, to);
    if(new){
        other->object = recipient;

        other = bst_alloc_node(((connections_t*)recipient->object)->receiving, from);
        other->object = sender;
        bst_insert(((connections_t*)recipient->object)->receiving, other);
        ((connections_t*)recipient->object)->receiving_count++;

        if(!relation->record.dirty && ((connections_t*)recipient->object)->receiving_count >= relation->record.max){
            if(((connections_t*)recipient->object)->receiving_count > relation->record.max){
                relation->record.max = ((connections_t*)recipient->object)->receiving_count;
                relation->record.len = 0;
            }
            relation->record.len++;
            relation->record.champions[relation->record.len-1] = recipient;
            insertion_sort(relation->record.champions, relation->record.len);
        }
    }
}

void update_all_giving_recursive(bst_t* tree, bst_node_t* node, char* from, bst_node_t* to){
    if(node != tree->NIL){
        update_all_giving_recursive(tree, node->left, from, to);
        update_all_giving_recursive(tree, node->right, from, to);

        bst_node_t* other = node->object;
        bst_node_t* connection = bst_get(((connections_t*)other->object)->receiving, from);
        connection->object = to;
    }
}

void update_all_receiving_recursive(bst_t* tree, bst_node_t* node, char* from, bst_node_t* to){
    if(node != tree->NIL){
        update_all_receiving_recursive(tree, node->left, from, to);
        update_all_receiving_recursive(tree, node->right, from, to);

        bst_node_t* other = node->object;
        bst_node_t* connection = bst_get(((connections_t*)other->object)->giving, from);
        connection->object = to;
    }
}

void free_connections(relation_t* relation, bst_node_t* target){
    free(((connections_t*)target->object)->giving);
    free(((connections_t*)target->object)->receiving);
    free((connections_t*)target->object);
    bst_node_t* tbd = bst_remove(relation->connections_tree, target);
    if(tbd != target){ // bst's...
        //target holds the moved node
        //tbd holds the node that we need to search for
        update_all_receiving_recursive(((connections_t*)target->object)->receiving, 
                                ((connections_t*)target->object)->receiving->root, /*from*/ tbd->key, /*to*/ target);
        update_all_giving_recursive(((connections_t*)target->object)->giving,
                                ((connections_t*)target->object)->giving->root, /*from*/ tbd->key, /*to*/ target);
        relation->record.dirty = 1;
    }
    free(tbd);
}

int has_connections(bst_node_t* target){
    return ((connections_t*)target->object)->giving->root != ((connections_t*)target->object)->giving->NIL ||
           ((connections_t*)target->object)->receiving->root != ((connections_t*)target->object)->receiving->NIL;
}

void del_connection(bst_t* relations_tree, char* rel_id, char* from, char* to){
    bst_node_t* relation_node = bst_get(relations_tree, rel_id);
    if(relation_node != relations_tree->NIL){
        relation_t* relation = relation_node->object;
        bst_node_t *sender, *recipient;
        sender = bst_get(relation->connections_tree, from);
        if(sender != relation->connections_tree->NIL){
            bst_node_t *sender_connection, *recipient_connection;
            sender_connection = bst_get(((connections_t*)sender->object)->giving, to);
            if(sender_connection != ((connections_t*)sender->object)->giving->NIL){
                recipient = sender_connection->object;
                recipient_connection = bst_get(((connections_t*)recipient->object)->receiving, from); // If this is NIL something bad happened...

                free(bst_remove(((connections_t*)sender->object)->giving, sender_connection));
                free(bst_remove(((connections_t*)recipient->object)->receiving, recipient_connection));
                if(((connections_t*)recipient->object)->receiving_count == relation->record.max)
                    relation->record.dirty = 1;

                ((connections_t*)recipient->object)->receiving_count--;

                if(!has_connections(recipient))
                    free_connections(relation, recipient);
            }
        }
    }
}

void del_all_giving(relation_t* relation, bst_t* giving_tree, char* id){
    bst_node_t* node;
    while(giving_tree->root != giving_tree->NIL){
        node = giving_tree->root;

        bst_node_t* recipient = node->object;
        bst_node_t* recipient_connection = bst_get(((connections_t*)recipient->object)->receiving, id);
        free(bst_remove(((connections_t*)recipient->object)->receiving, recipient_connection));
        free(bst_remove(giving_tree, node));

        if(relation->record.max == ((connections_t*)recipient->object)->receiving_count)
            relation->record.dirty = 1;

        ((connections_t*)recipient->object)->receiving_count--;
        if(strcmp(recipient->key, id) != 0 && !has_connections(recipient))
            free_connections(relation, recipient);
    }
}

void del_all_receiving(relation_t* relation, bst_t* receiving_tree, char* id){
    bst_node_t* node;
    while(receiving_tree->root != receiving_tree->NIL){
        node = receiving_tree->root;

        bst_node_t* sender = node->object;
        bst_node_t* sender_connection = bst_get(((connections_t*)sender->object)->giving, id);
        free(bst_remove(((connections_t*)sender->object)->giving, sender_connection));
        free(bst_remove(receiving_tree, node));

        if(relation->record.max == ((connections_t*)sender->object)->receiving_count)
            relation->record.dirty = 1;

        if(strcmp(sender->key, id) != 0 && !has_connections(sender))
            free_connections(relation, sender);
    }
}

void del_entity_from_relation_recursive(bst_t* relations_tree, bst_node_t* relation_node, char* id){
    if(relation_node != relations_tree->NIL){
        del_entity_from_relation_recursive(relations_tree, relation_node->left, id);
        del_entity_from_relation_recursive(relations_tree, relation_node->right, id);

        relation_t* relation = relation_node->object;
        bst_node_t* target = bst_get(relation->connections_tree, id);
        if(target != relation->connections_tree->NIL){
            if(relation->record.max == ((connections_t*)target->object)->receiving_count)
                relation->record.dirty = 1;

            del_all_giving(relation, ((connections_t*)target->object)->giving, id);
            // target may have moved as a result of some other deletion in the tree
            target = bst_get(relation->connections_tree, id);
            del_all_receiving(relation, ((connections_t*)target->object)->receiving, id);
            // target may have moved as a result of some other deletion in the tree
            target = bst_get(relation->connections_tree, id);

            free_connections(relation, target);
        }
    }
}

void del_entity(ht_t tracked, bst_t* relations_tree, char* id){
    // forall relations in relations_tree => delete id's connections
    del_entity_from_relation_recursive(relations_tree, relations_tree->root, id);

    ht_delete(tracked, id);
}

void tree_walk_for_max_connections(relation_t* relation, bst_t* tree, bst_node_t* node){
    if(node != tree->NIL){
        tree_walk_for_max_connections(relation, tree, node->left);

        connections_t* connections = node->object;
        int count = connections->receiving_count;
        if(count > 0){
            if(count == relation->record.max){
                relation->record.len++;
                relation->record.champions[relation->record.len-1] = node;
            } else if(count > relation->record.max){
                relation->record.max = count;
                relation->record.len = 1;
                relation->record.champions[relation->record.len-1] = node;
            }
        }

        tree_walk_for_max_connections(relation, tree, node->right);
    }
}

void compute_champions(relation_t* relation){
    relation->record.max = 0;
    relation->record.len = 0;
    tree_walk_for_max_connections(relation, relation->connections_tree, relation->connections_tree->root);
}

// Helper for report()
int g_found_first = 0;
int report_node_inorder(bst_t* tree, bst_node_t* relation_node){
    int any = 0;
    if(relation_node != tree->NIL){
        any |= report_node_inorder(tree, relation_node->left);

        relation_t* relation = relation_node->object;
        if(relation->record.dirty){
            compute_champions(relation);
            relation->record.dirty = 0;
        }

        if(relation->record.len > 0){
            // Print
            if(g_found_first)
                fputs(" ", stdout);

            fputs("\"", stdout);
            fputs(relation_node->key, stdout);
            fputs("\" ", stdout);
            any = 1;
            for(int i = 0; i < relation->record.len; i++){
                fputs("\"", stdout);
                fputs(relation->record.champions[i]->key, stdout);
                fputs("\" ", stdout);
            }
            fprintf(stdout, "%d;", relation->record.max);

            g_found_first = 1;
        }
        any |= report_node_inorder(tree, relation_node->right);
    }
    return any;
}

void report(bst_t* relations_tree){
    g_found_first = 0;
    int any = report_node_inorder(relations_tree, relations_tree->root);
    if(!any)
        printf("none");
    printf("\n");
}

relation_t* alloc_relation(){
    relation_t* rel = malloc(sizeof(relation_t));
    rel->record.max = 0;
    rel->record.len = 0;
    rel->record.dirty = 0;
    rel->connections_tree = bst_create();
    return rel;
}

connections_t* alloc_connections(){
    connections_t* conn = malloc(sizeof(connections_t));
    conn->giving = bst_create();
    conn->receiving = bst_create();
    conn->receiving_count = 0;
    return conn;
}

// This will always be O(n) time, since the array is always almost-sorted except the last element
// By chosing the last element as the pivot, we can linearly sort the array.
void insertion_sort(bst_node_t* arr[], int len){
    int i = len - 1;
    bst_node_t* tmp;
    while(i > 0 && strcmp(arr[i-1]->key, arr[i]->key) > 0){
        tmp = arr[i];
        arr[i] = arr[i-1];
        arr[i-1] = tmp;
        i--;
    }
}