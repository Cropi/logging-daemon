#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

typedef struct list {
    struct element *head;
} tList;

typedef struct element {
    int counter;
    char *messageHead;
    char *messageBody;
    struct element *next;
} tElem;

void listInit(tList *l);
void insertFirst(tList *l,  char *message);
void destroyList(tList *l);
tElem * search(tList *l, char *message, int index);
tElem *getMostPopular(tList *l);
void print(tList *l);

#endif
