list_item_t* listht_get_entity_item(listht_t* list, char* key){
    int idx = strhash(key);
    ht_entry_t* walk = list->ht->arr[idx];
    while(walk){
        list_item_t* list_item = (list_item_t*)walk->object;
        entity_t* entity = (entity_t*)list_item->object;

        if(!strcmp(entity->id, key)) return list_item;
        walk = walk->next;
    }
    return NULL;
}

void listht_add_entity_unique(listht_t* list, entity_t* entity){
    // List insertion
    list_item_t* new_item;

    new_item = malloc(sizeof(list_item_t));
    new_item->object = (void*)entity;
    new_item->prev = NULL;
    new_item->next = list->head;

    if(list->head != NULL)
        list->head->prev = new_item;

    list->head = new_item;

    // HT insertion
    int idx = strhash(entity->id);
    ht_entry_t* ht_entry = malloc(sizeof(ht_entry_t));
    ht_entry->object = (void*)new_item;
    ht_entry->next = list->ht->arr[idx];
    list->ht->arr[idx] = ht_entry;
}

int listht_add_entity(listht_t* list, entity_t* entity){
    if(listht_get_entity_item(list, entity->id) == NULL){
        listht_add_entity_unique(list, entity);
        return 1;
    }
    return 0;
}

void print_list(listht_t* list){
    if(list == NULL)
        return;

    list_item_t* walk = list->head;
    while(walk){
        printf("%s\n", ((entity_t*)walk->object)->id);
        walk = walk->next;
    }
}

void listht_free_entity_item(listht_t* list, list_item_t* item){
    // Adjust pointers
    if(item->prev == NULL)
        list->head = item->next;
    else
        item->prev->next = item->next;

    if(item->next != NULL)
        item->next->prev = item->prev;

    // Free ht entry
    int idx = strhash(((entity_t*)item->object)->id);
    ht_entry_t* walk = list->ht->arr[idx];
    if(walk->object == item){
        list->ht->arr[idx] = walk->next;
        free(walk);
    } else {
        while(walk->next){
            if(walk->next->object == item) {
                ht_entry_t* tbd = walk->next;
                walk->next = walk->next->next;
                free(tbd);
                break;
            }
            walk = walk->next;
        }
    }

    free(item);
}

int listht_free_entity(listht_t* list, char* id){
    list_item_t* item = listht_get_entity_item(list, id);
    if(item != NULL) {
        listht_free_entity_item(list, item);
        return 1;
    }
    return 0;
}

void free_entities_listht(listht_t* list){
    if(list == NULL)
        return;

    list_item_t* walk = list->head;
    list_item_t* tmp;

    while(walk){
        // Free the whole ht row, since we're deleting everything anyways.
        ht_free_row(list->ht, ((entity_t*)walk->object)->id);

        tmp = walk->next;
        free(walk);
        walk = tmp;
    }

    free_ht(list->ht);
    free(list);
}
