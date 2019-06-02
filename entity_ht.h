/* Entity hash table */
void ht_add_entity_unique(ht_t* table, entity_t* entity){
    int idx = strhash(entity->id);
    ht_entry_t* entry = malloc(sizeof(ht_entry_t));
    entry->object = (void*)entity;
    entry->next = table->arr[idx];
    table->arr[idx] = entry;
}

entity_t* ht_get_entity(ht_t* table, char* key){
    int idx = strhash(key);
    ht_entry_t* walk = table->arr[idx];
    while(walk){
        if(!strcmp(((entity_t*)walk->object)->id, key))
            return (entity_t*)walk->object;
        walk = walk->next;
    }
    return NULL;
}

void ht_free_entity(ht_t* table, char* id){
    int idx = strhash(id);
    if(table->arr[idx] != NULL){
        ht_entry_t* walk = table->arr[idx];
        while(walk->next){
            if(!strcmp(((entity_t*)walk->next->object)->id, id)){
                ht_entry_t* tbd = walk->next;
                walk->next = walk->next->next;
                free(tbd);
                return;
            }
            walk = walk->next;
        }
    }
}
