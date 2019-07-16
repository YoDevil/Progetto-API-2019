#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "entity_ht.h"
#include "relation_bstht.h"
#include "entity_listht.h"
#include "connections_bstht.h"

void add_entity(ht_t*, char*);
entity_t* get_entity(ht_t*, char*);
void del_entity(ht_t*, bstht_t*, char*);
void add_connection(bstht_t*, char*, entity_t*, entity_t*);
relation_t* get_relation(bstht_t*, char*);
connections_t* get_connections(relation_t*, char*);
void del_connection(relation_t*, char*, char*);
void report(bstht_t* tree);

int main(int argc, char** argv){
    char cmd[ID_LEN+1];
    char arg1[ID_LEN+1];
    char arg2[ID_LEN+1];
    char arg3[ID_LEN+1];
    int ok;
    ht_t* entities_table = ht_create(BIG_HASH_TABLE_SIZE);
    bstht_t* relations_tree = bstht_create(BIG_HASH_TABLE_SIZE);

    while(1){
        ok = scanf("%s", cmd);

        if (ok && !strcmp(cmd, "addent")){
            ok = scanf(" \"%[^\"]\"", arg1);
            add_entity(entities_table, arg1);

        } else if(ok && !strcmp(cmd, "delent")){
            ok = scanf(" \"%[^\"]\"", arg1);

            del_entity(entities_table, relations_tree, arg1);

        } else if(ok && !strcmp(cmd, "addrel")){
            ok = scanf(" \"%[^\"]\" \"%[^\"]\" \"%[^\"]\"", arg1, arg2, arg3);

            entity_t* entity1 = get_entity(entities_table, arg1);
            entity_t* entity2 = get_entity(entities_table, arg2);

            if(entity1 && entity2)
                add_connection(relations_tree, arg3, entity1, entity2);

        } else if(ok && !strcmp(cmd, "delrel")){
            ok = scanf(" \"%[^\"]\" \"%[^\"]\" \"%[^\"]\"", arg1, arg2, arg3);

            relation_t* relation = get_relation(relations_tree, arg3);
            if(relation)
                del_connection(relation, arg1, arg2);

        } else if(ok && !strcmp(cmd, "report")){
            report(relations_tree);
        } else if(ok && !strcmp(cmd, "end")){
            break;
        }

        if(!ok)
            abort();
    }

    return 0;
}

void add_entity(ht_t* table, char* id){
    if(ht_get_entity(table, id) == NULL){
        entity_t* entity = malloc(sizeof(entity_t));
        strcpy(entity->id, id);
        ht_add_entity_unique(table, entity);
    }
}

entity_t* get_entity(ht_t* table, char* key){
    return ht_get_entity(table, key);
}

int cleanup_connections(bstht_t* tree, bst_node_t* connections_node, bst_node_t** update_me){
    connections_t* connections = (connections_t*)connections_node->object;
    if(connections->receiving->head == NULL && connections->giving->head == NULL){
        bstht_free_connections_node(tree, connections_node, update_me);
        return 1;
    }
    return 0;
}

// Helper for del_entity()
void del_entity_from_relation_recursive(bst_node_t* relation_node, char* id){
    if(relation_node) {
        relation_t* relation = (relation_t*)relation_node->object;

        // Delete all its connections
        bst_node_t* my_connections_node = bstht_get_connections_node(relation->connections, id);
        if(my_connections_node){
            connections_t* my_connections = (connections_t*)my_connections_node->object;
            bst_node_t* other_connections_node;
            connections_t* other_connections;

            list_item_t* walk = my_connections->giving->head;
            while(walk){    // Walk double linked list
                entity_t* to = (entity_t*)walk->object;
                other_connections_node = bstht_get_connections_node(relation->connections, to->id);
                other_connections = (connections_t*)other_connections_node->object;
                other_connections->receiving_count--;
                listht_free_entity(other_connections->receiving, id); // Remove me from other's receiving list
                int freed = cleanup_connections(relation->connections, other_connections_node, &my_connections_node); // If is not giving or receiving anything, free it
                if(!freed)
                    bstht_update_connections_node(relation->connections, other_connections_node, &my_connections_node); // Reorder
                walk = walk->next;
            }

            walk = my_connections->receiving->head;
            while(walk){    // Walk double linked list
                entity_t* from = (entity_t*)walk->object;
                other_connections_node = bstht_get_connections_node(relation->connections, from->id);
                other_connections = (connections_t*)other_connections_node->object;
                listht_free_entity(other_connections->giving, id); // Remove me from other's giving list
                cleanup_connections(relation->connections, other_connections_node, &my_connections_node); // If is not giving or receiving anything, free it
                walk = walk->next;
            }

            bstht_free_connections_node(relation->connections, my_connections_node, NULL);
        }

        del_entity_from_relation_recursive(relation_node->left, id);
        del_entity_from_relation_recursive(relation_node->right, id);
    }
}

void del_entity(ht_t* entity_table, bstht_t* relation_tree, char* id){
    // For each relation
    del_entity_from_relation_recursive(relation_tree->root, id);

    // Delete from ht
    ht_free_entity(entity_table, id);
}

void add_connection(bstht_t* tree, char* id, entity_t* from, entity_t* to){
    relation_t* relation;
    int unique = 0;

    bst_node_t* bst_node = bstht_get_relation_node(tree, id);
    if(bst_node == NULL){
        relation = malloc(sizeof(relation_t));
        strcpy(relation->id, id);
        relation->connections = bstht_create(BIG_HASH_TABLE_SIZE);
        bstht_add_relation_unique(tree, relation);
    } else {
        relation = (relation_t*)bst_node->object;
    }

    // Get or create "from" connections
    bst_node_t* from_connections_node = bstht_get_connections_node(relation->connections, from->id);
    connections_t* from_connections;
    if(from_connections_node == NULL){
        from_connections = malloc(sizeof(connections_t));
        from_connections->me = from;
        from_connections->giving = listht_create(SMALL_HASH_TABLE_SIZE);
        from_connections->receiving = listht_create(SMALL_HASH_TABLE_SIZE);
        from_connections->receiving_count = 0;
        from_connections_node = bstht_add_connections_unique(relation->connections, from_connections);
    } else {
        from_connections = (connections_t*)from_connections_node->object;
    }

    // Get or create "to" connections
    bst_node_t* to_connections_node = bstht_get_connections_node(relation->connections, to->id);
    connections_t* to_connections;
    if(to_connections_node == NULL){
        to_connections = malloc(sizeof(connections_t));
        to_connections->me = to;
        to_connections->giving = listht_create(SMALL_HASH_TABLE_SIZE);
        to_connections->receiving = listht_create(SMALL_HASH_TABLE_SIZE);
        to_connections->receiving_count = 1;
        to_connections_node = bstht_add_connections_unique(relation->connections, to_connections);
        unique = 1;
    } else {
        to_connections = (connections_t*)to_connections_node->object;
    }

    // Add to the lists
    if(unique){
        // We just created a connections_t entry for 'to'
        // The connection is unique and the node is already in place with receiving_count=1
        listht_add_entity_unique(from_connections->giving, to); // Add "to" to "from"'s giving list
        listht_add_entity_unique(to_connections->receiving, from); // Add "from" to "to"'s receiving list
    } else {
        // Check if the connection is unique or a duplicate
        unique = listht_add_entity(from_connections->giving, to);
        if(unique){
            listht_add_entity_unique(to_connections->receiving, from);
            to_connections->receiving_count++;
            bstht_update_connections_node(relation->connections, to_connections_node, NULL); // Reorder
        }
    }
}

void del_connection(relation_t* relation, char* from, char* to){
    bst_node_t* from_connections_node = bstht_get_connections_node(relation->connections, from);
    if(from_connections_node != NULL){
        bst_node_t* to_connections_node = bstht_get_connections_node(relation->connections, to);
        if(to_connections_node != NULL){
            connections_t* from_connections = (connections_t*)from_connections_node->object;
            connections_t* to_connections = (connections_t*)to_connections_node->object;
            int existed = listht_free_entity(from_connections->giving, to);
            if(existed){
                listht_free_entity(to_connections->receiving, from);
                to_connections->receiving_count--;

                int is_loop = from_connections_node == to_connections_node;
                int freed = cleanup_connections(relation->connections, from_connections_node, &to_connections_node);
                if(!is_loop)
                    freed = cleanup_connections(relation->connections, to_connections_node, NULL);

                if(!freed)
                    bstht_update_connections_node(relation->connections, to_connections_node, NULL);
            }
        }
    }
}

int g_found_first = 0;
// Helper for report()
int report_node(bst_node_t* relation_node){
    int any = 0;
    if(relation_node != NULL){
        any |= report_node(relation_node->left);

        relation_t* relation = (relation_t*)relation_node->object;
        bst_node_t* max_node = bst_get_max(relation->connections->root);
        if(max_node){
            any = 1;
            connections_t* best_connections = (connections_t*)max_node->object;
            int count = best_connections->receiving_count;

            // Print the relation id and the first entity id
            if(g_found_first)
                printf(" ");
            printf("\"%s\" \"%s\" ", relation->id, best_connections->me->id);

            // If there are more entities with the same number of connections, print them too
            bst_node_t* next_best_node = bst_get_predecessor(max_node);
            while(next_best_node && ((connections_t*)next_best_node->object)->receiving_count == count){
                printf("\"%s\" ", ((connections_t*)next_best_node->object)->me->id);
                next_best_node = bst_get_predecessor(next_best_node);
            }
            printf("%d;", count);

            g_found_first = 1;
        }
        any |= report_node(relation_node->right);
    }
    return any;
}

void report(bstht_t* tree){
    g_found_first = 0;
    int any = report_node(tree->root);
    if(!any)
        printf("none");
    printf("\n");
}

relation_t* get_relation(bstht_t* tree, char* id){
    bst_node_t* bst_node = bstht_get_relation_node(tree, id);
    if(bst_node)
        return (relation_t*)bst_node->object;
    return NULL;
}

connections_t* get_connections(relation_t* relation, char* id){
    bst_node_t* bst_node = bstht_get_connections_node(relation->connections, id);
    if(bst_node)
        return (connections_t*)bst_node->object;
    return NULL;
}
