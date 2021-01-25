#ifndef MENU_H
#define MENU_H

#define MENU_ITEM_NAME_MAX 16

typedef struct menu_item {
    char name[MENU_ITEM_NAME_MAX];
} menu_item_t;

void menu_application_start();

#endif
