#ifndef MENU_H
#define MENU_H

typedef struct menu_item{
    char name[16];
    void (*app_callback)(void);
} menu_item_t;

void menu_application_start();

#endif
