#ifndef MENU_H
#define MENU_H

#include <string.h>

#define MENU_ITEM_NAME_MAX 16

typedef struct menu_item {
    char name_data[MENU_ITEM_NAME_MAX];
    string_t name;
} menu_item_t;

void menu_application_start();

#endif
