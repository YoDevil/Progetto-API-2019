#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "ht.h"
#include "bst.h"

connections_t* alloc_connections();
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
        relation_node->object = bst_create();

    // Get sender and recipient
    bst_t* connections_tree = relation_node->object;
    bst_node_t* sender;
    new = bst_get_or_alloc_and_insert(&sender, connections_tree, from);
    if(new)
        sender->object = alloc_connections();
    bst_node_t* recipient;
    new = bst_get_or_alloc_and_insert(&recipient, connections_tree, to);
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
    }
}

void del_connection(bst_t* relations_tree, char* rel_id, char* from, char* to){
    bst_node_t* relation_node = bst_get(relations_tree, rel_id);
    if(relation_node != relations_tree->NIL){
        bst_t* connections_tree = relation_node->object;
        bst_node_t *sender, *recipient;
        sender = bst_get(connections_tree, from);
        if(sender != connections_tree->NIL){
            bst_node_t *sender_connection, *recipient_connection;
            sender_connection = bst_get(((connections_t*)sender->object)->giving, to);
            if(sender_connection != ((connections_t*)sender->object)->giving->NIL){
                recipient = sender_connection->object;
                recipient_connection = bst_get(((connections_t*)recipient->object)->receiving, from); // If this is NIL something bad happened...

                free(bst_remove(((connections_t*)sender->object)->giving, sender_connection));
                free(bst_remove(((connections_t*)recipient->object)->receiving, recipient_connection));
                ((connections_t*)recipient->object)->receiving_count--;
            }
        }
    }
}

void del_all_giving(bst_t* giving_tree, char* id){
    bst_node_t* node;
    while(giving_tree->root != giving_tree->NIL){
        node = giving_tree->root;

        bst_node_t* recipient = node->object;
        bst_node_t* recipient_connection = bst_get(((connections_t*)recipient->object)->receiving, id);
        free(bst_remove(((connections_t*)recipient->object)->receiving, recipient_connection));
        free(bst_remove(giving_tree, node));
        ((connections_t*)recipient->object)->receiving_count--;
        // TODO: Remove relation if empty
    }
}

void del_all_receiving(bst_t* receiving_tree, char* id){
    bst_node_t* node;
    while(receiving_tree->root != receiving_tree->NIL){
        node = receiving_tree->root;

        bst_node_t* sender = node->object;
        bst_node_t* sender_connection = bst_get(((connections_t*)sender->object)->giving, id);
        free(bst_remove(((connections_t*)sender->object)->giving, sender_connection));
        free(bst_remove(receiving_tree, node));
        // TODO: Remove entity if empty
    }
}

void del_entity_from_relation_recursive(bst_t* relations_tree, bst_node_t* relation_node, char* id){
    if(relation_node != relations_tree->NIL){
        del_entity_from_relation_recursive(relations_tree, relation_node->left, id);
        del_entity_from_relation_recursive(relations_tree, relation_node->right, id);

        bst_t* connections_tree = relation_node->object;
        bst_node_t* target = bst_get(connections_tree, id);
        if(target != connections_tree->NIL){
            del_all_giving(((connections_t*)target->object)->giving, id);
            del_all_receiving(((connections_t*)target->object)->receiving, id);

            ((connections_t*)target->object)->receiving_count = 0;
            // TODO: Remove id from connections_tree
        }
    }
}

void del_entity(ht_t tracked, bst_t* relations_tree, char* id){
    // forall relations in relations_tree => delete id's connections
    del_entity_from_relation_recursive(relations_tree, relations_tree->root, id);

    ht_delete(tracked, id);
}

bst_node_t* g_maximums[1000] = {0};
int g_len = 0;
int g_max = 0;
void tree_walk_for_max_connections(bst_t* tree, bst_node_t* node){
    if(node != tree->NIL){
        tree_walk_for_max_connections(tree, node->left);

        connections_t* connections = node->object;
        int count = connections->receiving_count;
        if(count > 0){
            if(count == g_max){
                g_len++;
                g_maximums[g_len-1] = node;
            } else if(count > g_max){
                g_max = count;
                g_len = 1;
                g_maximums[g_len-1] = node;
            }
        }

        tree_walk_for_max_connections(tree, node->right);
    }
}

// Helper for report()
int g_found_first = 0;
int report_node_inorder(bst_t* tree, bst_node_t* relation_node){
    int any = 0;
    if(relation_node != tree->NIL){
        any |= report_node_inorder(tree, relation_node->left);

        bst_t* connections_tree = relation_node->object;
        g_max = 0;
        g_len = 0;
        tree_walk_for_max_connections(connections_tree, connections_tree->root);
        if(g_len > 0){
            // Print
            if(g_found_first)
                fputs(" ", stdout);

            fputs("\"", stdout);
            fputs(relation_node->key, stdout);
            fputs("\" ", stdout);
            any = 1;
            for(int i = 0; i < g_len; i++){
                fputs("\"", stdout);
                fputs(g_maximums[i]->key, stdout);
                fputs("\" ", stdout);
            }
            fprintf(stdout, "%d;", g_max);

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

connections_t* alloc_connections(){
    connections_t* conn = malloc(sizeof(connections_t));
    conn->giving = bst_create();
    conn->receiving = bst_create();
    conn->receiving_count = 0;
    return conn;
}
