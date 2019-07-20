int strhash(const char* str) {
    size_t i, sum;
    sum = 0;
    for(i = 0; str[i] != '\0'; i++)
        sum += (str[i] - '-' + 1) * (i + 1);

    return sum % HT_SIZE;
}

int ht_search(ht_t ht, char* key){
    int idx = strhash(key);
    ht_entry_t* walk = ht[idx];
    while(walk != NULL){
        if(!strcmp(walk->key, key))
            return 1;
        walk = walk->next;
    }
    return 0;
}

void ht_insert(ht_t ht, char* key){
    int idx = strhash(key);
    ht_entry_t* new = malloc(sizeof(ht_entry_t));
    strcpy(new->key, key);
    new->next = ht[idx];
    ht[idx] = new;
}

void ht_delete(ht_t ht, char* key){
    int idx = strhash(key);
    ht_entry_t* tbd;
    ht_entry_t* walk;

    if(ht[idx] == NULL)
        return;

    if(!strcmp(ht[idx]->key, key)){
        tbd = ht[idx];
        ht[idx] = tbd->next;
        free(tbd);
    } else {
        walk = ht[idx];
        while(walk->next != NULL){
            if(!strcmp(walk->next->key, key)){
                ht_entry_t* tbd = walk->next;
                walk->next = tbd->next;
                free(tbd);
                return;
            }
            walk = walk->next;
        }
    }
}
