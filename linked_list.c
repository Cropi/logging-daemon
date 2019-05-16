#include "linked_list.h"

void listInit(tList *l) {
    l->head = NULL;
}

void insertFirst(tList *l, char *message) {
    /*
     * If the string exists then only increment its count
     */
    tElem *actual = search(l, message);
    if (actual) {
        actual->counter++;
        return;
    }

    /*
     * If not exists allocate space for its data and the structure
     */
    tElem *newElem = (tElem *) malloc(sizeof(tElem));
    if (newElem == NULL) {
        fprintf(stderr, "WARNING: Could not allocate space for another message\n");
        return ;
    }

    newElem->counter = 1;
    newElem->message = (char *) malloc(sizeof(char) * (strlen(message) + 1));
    if (newElem->message == NULL) {
        fprintf(stderr, "WARNING: Could not allocate space for another message\n");
        free(newElem);
        return ;
    }
    strcpy(newElem->message, message);
    newElem->next = l->head;
    l->head = newElem;
}

void destroyList(tList *l) {
    tElem *tmp;
    while((tmp = l->head) != NULL) {
        l->head = l->head->next;
        free(tmp->message);
        free(tmp);
    }

}

tElem * search(tList *l, char *message) {
    tElem *tmp = l->head;
    while(tmp != NULL) {
        if (strcmp(tmp->message, message) == 0)
            return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

tElem *getMostPopular(tList *l) {
    tElem *tmp = l->head;
    tElem *ret = tmp;
    while(tmp != NULL) {
        if (tmp->counter > ret->counter)
            ret = tmp;

        tmp = tmp->next;
    }

    return ret;
}

void print(tList *l) {
    tElem *tmp = l->head;
    while(tmp != NULL) {
        printf("%2d: %s\n", tmp->counter, tmp->message);
        tmp = tmp->next;
    }
}
