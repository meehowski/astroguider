/*
* IncFile1.h
*
* Created: 29/09/2012 4:17:54 PM
*  Author: michal
*/


#ifndef MENU_H_
#define MENU_H_

#include "project.h"

// menu creation macros
#define menuItemExitAllocCb(m,n,c,p)                   menu_item_alloc(m, n, MENUTYPE_EXIT, NULL, 0, 0, 0, c, p)
#define menuItemValueIntAllocCb(m,n,v,min,d,max,c,p)   menu_item_alloc(m, n, MENUTYPE_VALUE_INT, &v, min, max, 0, c, p)
#define menuItemValueShortAllocCb(m,n,v,min,d,max,c,p) menu_item_alloc(m, n, MENUTYPE_VALUE_SHORT, &v, min, max, 0, c, p)
#define menuItemValueByteAllocCb(m,n,v,min,d,max,c,p)  menu_item_alloc(m, n, MENUTYPE_VALUE_BYTE, &v, min, max, 0, c, p)
#define menuItemValueFPAllocCb(m,n,v,min,d,max,c,p)    menu_item_alloc(m, n, MENUTYPE_VALUE_FP, &v, min, max, 0, c, p)
#define menuItemHMSAllocCb(m,n,v,c,p)                  menu_item_alloc(m, n, MENUTYPE_HMS, &v, 0, CONST_SEC_PER_DAY - 1, 0, c, p)
#define menuItemDMSAllocCb(m,n,v,c,p)                  menu_item_alloc(m, n, MENUTYPE_DMS, &v, 0, CONST_ARCSEC_PER_360 - 1, 0, c, p)
#define menuItemClockAllocCb(m,n,v,c,p)                menu_item_alloc(m, n, MENUTYPE_TIME, &v, 0, CONST_SEC_PER_DAY - 1, 0, c, p)
#define menuItemDateAllocCb(m,n,v,c,p)                 menu_item_alloc(m, n, MENUTYPE_DATE, &v, 748464, 780828, 0, c, p) // year 2012-2099
#define menuItemSubmenuAllocCb(m,n,c,p)                menu_item_alloc(m, n, MENUTYPE_SUB, NULL, 0, 0, 0, c, p)
#define menuItemChoiceAllocCb(m,n,v,c,p)               menu_item_alloc(m, n, MENUTYPE_CHOICE, &v, 0, 0, 0, c, p)
#define menuItemDBEntryAllocCb(m,n,v,d,c,p)            menu_item_alloc(m, n, MENUTYPE_DB_ENTRY, &v, 0, 0, d, c, p)
#define menuItemTextFieldAllocCb(m,n,v,l,c,p)          menu_item_alloc(m, n, MENUTYPE_TEXT_FIELD, &v, 0, l, 0, c, p)

#define menuItemExitAlloc(m,n)                         menuItemExitAllocCb(m,n,NULL,0)
#define menuItemValueIntAlloc(m,n,v,min,d,max)         menuItemValueIntAllocCb(m,n,v,min,d,max,NULL,0)
#define menuItemValueShortAlloc(m,n,v,min,d,max)       menuItemValueShortAllocCb(m,n,v,min,d,max,NULL,0)
#define menuItemValueByteAlloc(m,n,v,min,d,max)        menuItemValueByteAllocCb(m,n,v,min,d,max,NULL,0)
#define menuItemValueFPAlloc(m,n,v,min,d,max)          menuItemValueFPAllocCb(m,n,v,min,d,max,NULL,0)
#define menuItemHMSAlloc(m,n,v)                        menuItemHMSAllocCb(m,n,v,NULL,0)
#define menuItemDMSAlloc(m,n,v)                        menuItemDMSAllocCb(m,n,v,NULL,0)
#define menuItemClockAlloc(m,n,v)                      menuItemClockAllocCb(m,n,v,NULL,0)
#define menuItemDateAlloc(m,n,v)                       menuItemDateAllocCb(m,n,v,NULL,0)
#define menuItemSubmenuAlloc(m,n)                      menuItemSubmenuAllocCb(m,n,NULL,0)
#define menuItemChoiceAlloc(m,n,v)                     menuItemChoiceAllocCb(m,n,v,NULL,0)
#define menuItemDBEntryAlloc(m,n,v,d)                  menuItemDBEntryAllocCb(m,n,v,d,NULL,0)
#define menuItemTextFielAlloc(m,n,v,l)                 menuItemTextFieldAllocCb(m,n,v,l,NULL,0)

// Storage for our menus, choices and lists
// We use this instead of dynamic memory allocation as it is not available
#define CONST_MAX_MENU_LISTS    20
#define CONST_MAX_MENUS       80
#define CONST_MAX_MENU_CHOICES    40

// cursor blinking pattern mask
#define CONST_MENU_BLINK_PATTERN    0x08 // blink pattern mask

// menu exit values
enum
{
  CONST_MENU_ACTIVE = 0,
  CONST_MENU_TERMINATE,
  CONST_MENU_TERMINATE_ALL
};

// maximum size of menu text entry
#define MENU_TEXTSIZE (LCD_WIDTH-1)

// LCD entry width
#define LCD_BUFSIZE (LCD_WIDTH+1)

// Different types of menus
typedef enum
{
  MENUTYPE_UNDEFINED = 0,
  MENUTYPE_VALUE_INT,   // signed 32bit
  MENUTYPE_VALUE_SHORT, // signed 16bit
  MENUTYPE_VALUE_BYTE,  // signed 8bit
  MENUTYPE_VALUE_FP,    // floating point, 64bit
  MENUTYPE_CHOICE,      // multiple choice list
  MENUTYPE_SUB,         // submenu
  MENUTYPE_DB_ENTRY,    // database
  MENUTYPE_HMS,         // hours, minute, second
  MENUTYPE_DMS,         // degree, arcminute, arcsecond
  MENUTYPE_TIME,        // current time
  MENUTYPE_DATE,        // current date
  MENUTYPE_TEXT_FIELD,  // editable text field
  MENUTYPE_EXIT         // exit flag
} menu_type_e;

// menu update directives
typedef enum
{
  CONST_MENU_UPDATE_FULL,
  CONST_MENU_UPDATE_PARTIAL,
  CONST_MENU_UPDATE_VALUE
} menu_update_e;

// Enums for button pushes in the menu state machine
typedef enum
{
  button_init = 0,
  button_wait_press,
  button_wait_release,
  button_process,
  button_process_done,
  button_wait_exit,
  button_wait_exit_all
} menu_state_e;

// menu storage structures

// A menu structure
typedef struct menu_entry_s
{
  // lcd display text
  char    menu_title[MENU_TEXTSIZE];
  // pointers to next and previous menus
  struct menu_entry_s   *next;
  struct menu_entry_s   *prev;
  // the type of this menu
  menu_type_e     menu_type;
  // value storage pointer (also used as index for choice menus)
  int32_t     *value;
  // minimum value of a "value" menu
  int32_t     min;
  // maximum value of a "value" menu
  int32_t     max;
  // db id for this menu value;
  uint8_t   db_id;
  // callback function
  uint8_t (*callback)();
  int32_t     callback_arg;
  // list of choices
  uint8_t     cnum;
  struct clist_s  *cfirst;
  struct clist_s  *cend;
  // pointer to a menu list - a sub menu
  struct menu_list_s  *list;
} menu_entry_t ;

// A menu list, holds a double linked list of menus
typedef struct menu_list_s
{
  // number of menus in this list
  int8_t     num;
  // pointer to first menu
  menu_entry_t    *first;
  // pointer to last menu
  menu_entry_t    *last;
  // pointer to current menu
  menu_entry_t    *current;
} menu_list_t ;

// A choice structure
typedef struct clist_s
{
  // name of choice
  char    choice_name[MENU_TEXTSIZE];
  // pointer to next choice
  struct clist_s *next;
  // pointer to previous choice
  struct clist_s *prev;
} clist_t;

// DB_ENTRY choice value
typedef struct
{
  int32_t db_id;
  int32_t obj_id;
} menu_db_t ;

// functions
void menu_init();
int8_t menu_run(menu_list_t *list, menu_entry_t *init, void (*idle_callback)(void));
uint8_t menu_active(void);
//uint32_t menu_checksum_get(menu_list_t *m);
menu_list_t *menu_list_alloc(void);
int8_t menu_list_add_menu(menu_list_t *list, menu_entry_t *m);
menu_entry_t *menu_item_alloc(menu_list_t *list, const char *name, menu_type_e type, void *value, int32_t min, int32_t max, uint8_t db_id, uint8_t (*callback)(), int32_t parameter);
clist_t *menu_choice_alloc(menu_entry_t *m, const char *name);

#endif /* MENU_H_ */
