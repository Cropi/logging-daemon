#include "linked_list.h"

void listInit(tList *l) {
    l->head = NULL;
}

/* The algorithm of our insert function is the following:
 * 1. Divide the logging message into head and body
 * 2. Check if a message with the corresponding body exists
 *      If yes => increment its counter
 *             => exit function
 * 3.   If not => Allocate space for the entire structure + messageHead + messageBody
 * 4. Modify the linked list
 */
void insertFirst(tList *l, char *message) {

    int whiteSpaceCount = 0, index;
    for(index = 0; index < strlen(message) && whiteSpaceCount != 3; index++) {
        if (message[index] == '\0') {
            fprintf(stderr, "WARNING: Invalid logger message syntax\n");
            return;
        }
        else if (message[index] == ' ')
            whiteSpaceCount++;
    }

    /*
     * If the string exists then only increment its count
     */
    tElem *actual = search(l, message, index);
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

    /* Set up counter */
    newElem->counter = 1;
    /* Allocate space for message header */
    newElem->messageHead = (char *) malloc(sizeof(char) * index);
    if (newElem->messageHead == NULL) {
        fprintf(stderr, "WARNING: Could not allocate space for another message\n");
        free(newElem);
        return ;
    }
    strncpy(newElem->messageHead, message, index);
    newElem->messageHead[index-1] = 0;

    /* Allocate space for message body */
    newElem->messageBody = (char *) malloc(sizeof(char) * (strlen(message) - index + 1));
    if (newElem->messageBody == NULL) {
        fprintf(stderr, "WARNING: Could not allocate space for another message\n");
        free(newElem->messageHead);
        free(newElem);
        return ;
    }
    strncpy(newElem->messageBody, message+index, strlen(message) - index);
    newElem->messageBody[strlen(message) - index] = '\0';

    /* Set up the new element as head of the list */
    newElem->next = l->head;
    l->head = newElem;
}

/*
 * Dealloc the elements of the list
 */
void destroyList(tList *l) {
    tElem *tmp;
    while((tmp = l->head) != NULL) {
        l->head = l->head->next;
        free(tmp->messageHead);
        free(tmp->messageBody);
        free(tmp);
    }

}

/*
 * Check if an element with content of message exists within the list
 */
tElem * search(tList *l, char *message, int index) {
    tElem *tmp = l->head;
    while(tmp != NULL) {
        if (strcmp(tmp->messageBody, message+index) == 0)
            return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

/*
 * Return an element with the largest occurence
 */
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

/*
 * Only for debugging purpose. Prints elements of a list.
 */
void print(tList *l) {
    tElem *tmp = l->head;
    while(tmp != NULL) {
        printf("%2d: %s %s", tmp->counter, tmp->messageHead, tmp->messageBody);
        tmp = tmp->next;
    }
}
