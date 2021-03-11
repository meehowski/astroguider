/*
* CFile1.c
*
* Created: 22/09/2012 3:12:33 PM
*  Author: michal
*/

#include "project.h"

// Pools for the menus, choices and lists
// We use this instead of dynamic memory allocation as it is not available
static menu_list_t    list_pool[CONST_MAX_MENU_LISTS];
static menu_entry_t   menu_pool[CONST_MAX_MENUS];
static clist_t        choice_list_pool[CONST_MAX_MENU_CHOICES];

// Indexes for next item to be allocated
static int8_t     nl = 0;
static int8_t     nm = 0;
static int8_t     nc = 0;

// Helper var for indexed (text field, f.ex.) editing
static int8_t     edit_index = 0;

// forward declarations
static void menu_display_update(menu_entry_t *m, menu_update_e what_to_update);

// variable representing menu completion value
uint8_t menu_retval = CONST_MENU_TERMINATE_ALL;

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*      Initialize and clear all menu resources                                */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
void menu_init(void)
{
  DEBUG_PRINT("MENU lib init\n");
  nl = 0;
  nm = 0;
  nc = 0;
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*      Create a new menu list                                                 */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
menu_list_t *menu_list_alloc(void)
{
  menu_list_t  *list;

  if (nl >= CONST_MAX_MENU_LISTS - 1)
  {
    RUNTIME_ERROR("NULL ALLOC");
    return (NULL);
  }

  list = & (list_pool[nl++]);
  list->first = NULL;
  list->last  = NULL;
  list->num   = 0;
  return (list);
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*      Add a menu to a menu list                                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
int8_t menu_list_add_menu(menu_list_t *list, menu_entry_t *m)
{
  menu_entry_t    *TopMenu;
  menu_entry_t    *EndMenu;

  // check for valid list
  if (list == NULL)
  {
    RUNTIME_ERROR("NULL LIST");
    return (-1);
  }

  // check for valid menu
  if (m == NULL)
  {
    RUNTIME_ERROR("NULL MENU");
    return (-2);
  }

  if (list->num == 0)
  {
    // first entry
    list->first = m;
    list->last  = m;
    // double linked list that we want to rate so point at ourself
    m->next = m;
    m->prev = m;
  }
  else
  {
    // get the menus at start and end of the list
    TopMenu = list->first;
    EndMenu = list->last;
    // Add to end of list
    m->prev       = EndMenu;
    m->next       = list->first;
    // relink the list
    EndMenu->next = m;
    TopMenu->prev = m;
    // this menu is now the last one
    list->last    = m;
  }

  // make new menu current menu
  list->current = list->first;
  // one more entry
  list->num++;
  return (list->num);
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*  Initialize a menu and link into the menu list                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
menu_entry_t *menu_item_alloc(menu_list_t *list, const char *name, menu_type_e type, void *value, int32_t min, int32_t max, uint8_t db_id, uint8_t (*callback)(), int32_t parameter)
{
  menu_entry_t    *m;
  //menu_db_t *menu_db_entry;

  // error checking
  if (((type == MENUTYPE_VALUE_INT)
       || (type == MENUTYPE_VALUE_SHORT)
       || (type == MENUTYPE_VALUE_BYTE)
       || (type == MENUTYPE_VALUE_FP)
       || (type == MENUTYPE_CHOICE)
       || (type == MENUTYPE_DB_ENTRY))
      && (value == NULL))
  {
    RUNTIME_ERROR("NULL ARGS");
    return NULL;
  }

  if (nm >= CONST_MAX_MENUS - 1)
  {
    RUNTIME_ERROR("NULL ALLOC");
    return (NULL);
  }

  if ((type == MENUTYPE_DB_ENTRY) && (db_id >= db_get_num()))
  {
    RUNTIME_ERROR("INVALID DB ID");
    return (NULL);
  }

  if ((type == MENUTYPE_DB_ENTRY) && ((max >= db_get_size(db_id)) || (min >= db_get_size(db_id))))
  {
    RUNTIME_ERROR("INVALID DB IVAL");
    return (NULL);
  }

  // We have no malloc so use allocate from an array
  m = &(menu_pool[nm++]);
  // Copy menu name
  strncpy(m->menu_title, name, MENU_TEXTSIZE);
  m->menu_title[MENU_TEXTSIZE - 1] = 0;
  // note menu type
  m->menu_type = type;

  // sanity checks
  if (type == MENUTYPE_VALUE_INT)
  {
    if ((* (int32_t *) value < min)
        || (* (int32_t *) value > max))
    {
      * (int32_t *) value = (min + max) / 2;
    }
  }
  else if (type == MENUTYPE_VALUE_SHORT)
  {
    if ((* (int16_t *) value < min)
        || (* (int16_t *) value > max))
    {
      * (int16_t *) value = (min + max) / 2;
    }
  }
  else if (type == MENUTYPE_VALUE_BYTE)
  {
    if ((* (int8_t *) value < min)
        || (* (int8_t *) value > max))
    {
      * (int8_t *) value = (min + max) / 2;
    }
  }
  else if (type == MENUTYPE_VALUE_FP)
  {
    if ((* (double *) value < min)
        || (* (double *) value > max))
    {
      * (double *) value = (min + max) / 2;
    }
  }
  else if (type == MENUTYPE_DATE)
  {
    if ((* (int32_t *) value < min)
        || (* (int32_t *) value > max))
    {
      * (int32_t *) value = min;
    }
  }

  // db id
  m->db_id = db_id;
  // default values
  m->value     = value; /* pointer assignment */
  m->min = min;
  m->max = max;

  // clear db entry if provided
  if (type == MENUTYPE_DB_ENTRY)
  {
    memset(value, 0, sizeof(menu_db_t));
  }

  // callback
  m->callback  = callback;
  m->callback_arg = parameter;
  // no choices yet
  m->cfirst = NULL;
  m->cend   = NULL;
  m->cnum   = 0;
  // no sub menu yet
  m->list = NULL;
  // link into list
  menu_list_add_menu(list, m);
  // return pointer to this menu
  return (m);
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*  Add one choice to a menu                                                   */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
clist_t *menu_choice_alloc(menu_entry_t *m, const char *name)
{
  clist_t   *c;
  clist_t   *end;

  // check for null pointer
  if (m == NULL)
  {
    RUNTIME_ERROR("NULL MENU");
    return (NULL);
  }

  if (nc >= CONST_MAX_MENU_CHOICES - 1)
  {
    RUNTIME_ERROR("NULL ALLOC");
    return (NULL);
  }

  // We have no malloc so use allocate from an array
  c = & (choice_list_pool[nc++]);

  // first choice
  if (m->cnum == 0)
  {
    m->cfirst = c;
    m->cend   = c;
    c->prev   = (clist_t *) NULL;
    c->next   = (clist_t *) NULL;
  }
  else
  {
    // subsequent choices
    end = m->cend;
    // only link to end - this list is not circular
    m->cend   = c;
    end->next = c;
    c->prev   = end;
    c->next   = (clist_t *) NULL;
  }

  // copy name for this choice
  strncpy(c->choice_name, name, MENU_TEXTSIZE);
  c->choice_name[MENU_TEXTSIZE - 1] = 0;
  // one more
  m->cnum++;
  // return pointer to the choice
  return (c);
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*      Update the LCD display with the menu information                       */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
static void menu_display_update(menu_entry_t *m, menu_update_e what_to_update)
{
  char buffer[LCD_BUFSIZE];
  uint8_t    i = 0;
  uint8_t icon;
  clist_t  *c  = NULL;
  db_entry_t *db_entry  = NULL;
  menu_db_t *menu_db_entry = NULL;
  int32_t clock;
  static uint8_t counter = 0;
  static int32_t clock_shadow = -1;

  // error checks
  if (m == NULL)
  {
    RUNTIME_ERROR("NULL MENU");
    return;
  }

  if (((m->menu_type == MENUTYPE_DB_ENTRY)
       || (m->menu_type == MENUTYPE_TEXT_FIELD))
      && (m->value == NULL))
  {
    RUNTIME_ERROR("M->VALUE NULL");
    return;
  }

  if (m->menu_type == MENUTYPE_DB_ENTRY)
  {
    // update db id if multiple menus share the same result structure
    menu_db_entry = (menu_db_t *) m->value;
    menu_db_entry->db_id = m->db_id;
  }

  // LCD line 1 display
  if (what_to_update == CONST_MENU_UPDATE_FULL)
  {
    //lcd_clear();
    clock_shadow = -1;
    counter = CONST_MENU_BLINK_PATTERN - 1;
    icon = CHR_EX;

    switch (m->menu_type)
    {
      case    MENUTYPE_DB_ENTRY:
        snprintf(buffer, LCD_BUFSIZE, "%c %-10s%4i", icon, m->menu_title, db_get_size(menu_db_entry->db_id));
        break;

      case  MENUTYPE_EXIT:
      case  MENUTYPE_SUB:
        icon = CHR_DNARROW;

      default:
        snprintf(buffer, LCD_BUFSIZE, "%c %-14s", icon, m->menu_title);
        break;
    }

    lcd_write_strn(0, 0, buffer, LCD_WIDTH);
  }

  // fill 2nd line
  switch (m->menu_type)
  {
    case    MENUTYPE_DB_ENTRY:
      menu_db_entry->obj_id = menu_db_entry->obj_id % db_get_size(menu_db_entry->db_id);
      db_entry = db_get_object(menu_db_entry->db_id, menu_db_entry->obj_id);

      if ((db_entry == NULL) || (db_entry->valid == 0))
      {
        lcd_write_strn(0, 1, "[INVALID]", LCD_WIDTH);
      }
      else
      {
        int32_t percentage = 100 * (menu_db_entry->obj_id + 1);
        percentage = percentage / db_get_size(menu_db_entry->db_id);
        snprintf(buffer, LCD_BUFSIZE, "%-11s %3i%%", db_entry->name, (int) percentage);
        lcd_write_strn(0, 1, buffer, LCD_WIDTH);
      }

      break;

    case    MENUTYPE_TEXT_FIELD:
      for (i = 0; i < m->max; i++)
      {
        if ((i == edit_index) && ((counter & CONST_MENU_BLINK_PATTERN) != 0))
        {
          lcd_write_char(i, 1, CHR_UNDERLINE);
        }
        else
        {
          lcd_write_char(i, 1, ((char *) m->value) [i]);
        }
      }

      //lcd_write_str(0, 1, (char *)m->value);
      break;

    case    MENUTYPE_SUB:
      lcd_write_strn(0, 1, "\333 ENTER MENU", LCD_WIDTH);
      break;

    case    MENUTYPE_EXIT:
      lcd_write_strn(0, 1, "\333 OK", LCD_WIDTH);
      break;

    case     MENUTYPE_TIME:
      clock = rtc_get();

      if (clock != clock_shadow)
      {
        clock_shadow = clock;
        snprintf(buffer, sizeof(buffer), "%3i%c %02i%c %02i%c",
                 (int) CONV_SEC_HR(clock), 'h',
                 (int) CONV_SEC_MIN(clock), 'm',
                 (int) CONV_SEC_SEC(clock), 's'
                );
        lcd_write_strn(0, 1, buffer, LCD_WIDTH);

        if (m->value != NULL)
        {
          * (m->value) = clock;
        }
      }

      break;

    case     MENUTYPE_DATE:
      snprintf(buffer, sizeof(buffer), "d:%02i m:%02i y:%04i",
               (int)((* (m->value) % 31) + 1),
               (int)(((* (m->value) / 31) % 12) + 1),
               (int)((* (m->value) / 372) % 9999)
              );
      lcd_write_strn(0, 1, buffer, LCD_WIDTH);
      break;

    case     MENUTYPE_HMS:
      snprintf(buffer, sizeof(buffer), "%3i%c %02i%c %02i%c",
               (int) CONV_SEC_HR(* (m->value)), 'h',
               (int) CONV_SEC_MIN(* (m->value)), 'm',
               (int) CONV_SEC_SEC(* (m->value)), 's'
              );
      lcd_write_strn(0, 1, buffer, LCD_WIDTH);
      break;

    case     MENUTYPE_DMS:
      snprintf(buffer, sizeof(buffer), "%3i%c %02i%c %02i%c",
               (int) CONV_ARCSEC_DEG(* (m->value)), CHR_DEGREE,
               (int) CONV_ARCSEC_ARCMIN(* (m->value)), CHR_ARCMINUTE,
               (int) CONV_ARCSEC_ARCSEC(* (m->value)), CHR_ARCSECOND
              );
      lcd_write_strn(0, 1, buffer, LCD_WIDTH);
      break;

    case    MENUTYPE_VALUE_INT:
      snprintf(buffer, LCD_BUFSIZE, "%-11i", (int) * ((int32_t *)m->value));
      lcd_write_strn(0, 1, buffer, LCD_WIDTH);
      break;

    case    MENUTYPE_VALUE_SHORT:
      snprintf(buffer, LCD_BUFSIZE, "%-11i", (int) * ((int16_t *)m->value));
      lcd_write_strn(0, 1, buffer, LCD_WIDTH);
      break;

    case    MENUTYPE_VALUE_BYTE:
      snprintf(buffer, LCD_BUFSIZE, "%-11i", (int) * ((int8_t *)m->value));
      lcd_write_strn(0, 1, buffer, LCD_WIDTH);
      break;

    case    MENUTYPE_VALUE_FP:
      snprintf(buffer, LCD_BUFSIZE, "%08.6f", *((double *)m->value));
      lcd_write_strn(0, 1, buffer, LCD_WIDTH);
      break;

    case    MENUTYPE_CHOICE:

      // NULL pointer checks
      if ((c = m->cfirst) == NULL)
      {
        RUNTIME_ERROR("NULL CPOINTER");
        break;
      }

      // move through list to find indexed choice
      // inefficient for a large number of choices
      for (i = 0; i < * (m->value); i++)
      {
        if (c->next != NULL)
        {
          c = c->next;
        }
      }

      snprintf(buffer, LCD_BUFSIZE, "%i %s", i + 1, c->choice_name);
      lcd_write_strn(0, 1, buffer, LCD_WIDTH);
      break;

    default:
      RUNTIME_DEBUG_VA("MENUTYPE %i", (int) m->menu_type);
      RUNTIME_ERROR("INVALID MENUTYPE");
      break;
  }

  counter++;
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*      Run a menu list                                                        */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

int8_t menu_run(menu_list_t *list, menu_entry_t *init, void (*idle_callback)())
{
  menu_state_e buttonState = button_init;
  uint16_t button = 0, accelerator2 = 0;
  uint8_t long_press_counter = 0, accelerator = 0; // multiThreshold = 1,
  //uint32_t delta = 1;
  menu_db_t *menu_db_entry = NULL;
  menu_retval = CONST_MENU_ACTIVE;

  // reset menu position
  if (init != NULL)
  {
    list->current = init;
  }

  lcd_clear();
  menu_display_update(list->current, CONST_MENU_UPDATE_FULL);  // FIXME double refresh

  while (1)
  {
    switch (buttonState)
    {
      case button_init:
        if (keypad_get() != kButtonNoKey)
        {
          break;
        }

        // continue
      case button_process_done:
        // Display select program
        menu_display_update(list->current, CONST_MENU_UPDATE_FULL);
        buttonState = button_wait_release;
        break;

      case button_wait_release:

        // Wait here until nothing is pressed
        // Read LCD buttons
        if ((long_press_counter < 20) && (keypad_get() != kButtonNoKey))
        {
          long_press_counter++;
          break;
        }

        buttonState = button_wait_press;
        break;

      case button_wait_press:
        // Wait here until nothing is pressed
        // Read buttons
        button = keypad_get();

        if (button == kButtonNoKey)
        {
          // reset long press processing
          long_press_counter = 0;

          //multiThreshold = 1;
          //delta = 1;
          // Update menu if needed
          if ((list->current->menu_type == MENUTYPE_TIME)
              || (list->current->menu_type == MENUTYPE_TEXT_FIELD))
          {
            menu_display_update(list->current, CONST_MENU_UPDATE_VALUE);
          }

          break;
        }

        buttonState = button_process;
        break;

      case button_process:
        /* analog accelerator. 1 on short press, actual analog 0-100 value on long press */
        accelerator = (long_press_counter == 0 ? 1 : abs(joystick_get(1)));
        /* square for larger increments */
        accelerator2 = (int16_t) accelerator * (int16_t) accelerator;

        switch (button)
        {
          case kButtonBLUE:
            // We are done
            menu_retval = CONST_MENU_TERMINATE_ALL;
            buttonState = button_wait_exit_all;
            break;

          case kButtonRED:
            buttonState = button_process_done;

            switch (list->current->menu_type)
            {
#ifdef ENTER_KEY_CHANGES_VALUES

              case   MENUTYPE_VALUE_INT:
              case   MENUTYPE_VALUE_SHORT:
              case   MENUTYPE_VALUE_BYTE:
              case   MENUTYPE_VALUE_FP:
              case   MENUTYPE_HMS:
              case   MENUTYPE_DMS:
                // simple menu with a variable
                // increase value
                * (list->current->value) = * (list->current->value) + accelerator;

                /* DELETEME
                if (multiThreshold < 10) {
                  multiThreshold++;
                } else if (delta < 100000000) {
                  delta = delta*10;
                  multiThreshold = 2;
                }
                */
                // check for max and wrap around
                if (* (list->current->value) > list->current->max)
                {
                  * (list->current->value) = list->current->min;
                  //delta=1;
                }

                break;

              case   MENUTYPE_CHOICE:

                // Menu with a list of choices
                // value is used as an index

                // check for end of choice list and wrap
                if (* (list->current->value) >= list->current->cnum - 1)
                {
                  * (list->current->value) = 0;
                }
                else
                {
                  * (list->current->value) = * (list->current->value) + 1;
                }

                break;
#endif // ENTER_KEY_CHANGES_VALUES

              case   MENUTYPE_EXIT:
                // An exit menu - we are done
                menu_retval = CONST_MENU_TERMINATE;
                buttonState = button_wait_exit;
                break;

              case   MENUTYPE_SUB:

                // selection using a sub menu
                if (list->current->list != NULL)
                {
                  // recursively enter menu run for sub menu
                  if (menu_run(list->current->list, NULL, idle_callback) == CONST_MENU_TERMINATE_ALL)
                  {
                    menu_retval = CONST_MENU_TERMINATE_ALL;
                    buttonState = button_wait_exit_all;
                  }
                }

                break;

              default:

                // exiting menu if callback is defined
                if (list->current->callback != NULL)
                {
                  menu_retval = CONST_MENU_TERMINATE;
                  buttonState = button_wait_exit;
                }

                break;
            } // menu_type

            // execute callback if defined
            if (list->current->callback != NULL)
            {
              lcd_clear();
              menu_retval = ((uint8_t ( *)(int32_t)) list->current->callback)(list->current->callback_arg);
            }

            break;

          case kButtonLeft:
          case kButtonPgLeft|kButtonLeft:
            switch (list->current->menu_type)
            {
              case MENUTYPE_TEXT_FIELD:
                if (edit_index > list->current->min)
                {
                  edit_index--;
                }

                break;

              default:

                // previous menu
                if (list->current->prev != NULL)
                {
                  list->current = list->current->prev;
                }

                break;
            }

            buttonState = button_process_done;
            break;

          case kButtonRight:
          case kButtonPgRight|kButtonRight:
            switch (list->current->menu_type)
            {
              case MENUTYPE_TEXT_FIELD:
                if (edit_index < list->current->max - 1)
                {
                  edit_index++;
                }

                break;

              default:

                // next menu
                if (list->current->next != NULL)
                {
                  list->current = list->current->next;
                }

                break;
            }

            buttonState = button_process_done;
            break;

          case kButtonUp:
          case kButtonPgUp|kButtonUp:
            buttonState = button_process_done;

            // execute callback if defined
            //if (list->current->callback != NULL) {
            //  done = ((uint8_t (*)(void))list->current->callback)();
            //}

            switch (list->current->menu_type)
            {
              case   MENUTYPE_TEXT_FIELD:
                if (list->current->value == NULL)
                {
                  RUNTIME_ERROR("L->C->V NULL");
                }

                if (((char *) list->current->value) [edit_index] < 127)
                {
                  ((char *) list->current->value) [edit_index]++;
                }

                break;

              case MENUTYPE_DB_ENTRY:
                menu_db_entry = (menu_db_t *) list->current->value;

                if (menu_db_entry == NULL)
                {
                  RUNTIME_ERROR("MT_DB_ENTRY NULL");
                  break;
                }

                // decrease value
                menu_db_entry->obj_id = menu_db_entry->obj_id - accelerator;

                if (menu_db_entry->obj_id < 0)
                {
                  menu_db_entry->obj_id = 0;
                }

                break;

              case   MENUTYPE_TIME:
                rtc_set(modulus_i(rtc_get() + accelerator2, CONST_SEC_PER_DAY));
                break;

              case   MENUTYPE_VALUE_FP:
                *((double *)list->current->value) += (((double)accelerator2) * ((double)accelerator2) / 1e6);

                if (* ((double *)list->current->value) > list->current->max)
                {
                  * ((double *)list->current->value) = list->current->max;
                  //delta=1;
                }

                break;

              case   MENUTYPE_VALUE_INT:
              case   MENUTYPE_VALUE_SHORT:
              case   MENUTYPE_VALUE_BYTE:
              case   MENUTYPE_DMS:
              case   MENUTYPE_HMS:
              case   MENUTYPE_DATE:
                // simple menu with a variable
                // increase value
                *(list->current->value) += accelerator2;

                /* DELETEME
                if (multiThreshold < 10) {
                  multiThreshold++;
                } else if (delta < 100000000) {
                  delta = delta*10;
                  multiThreshold = 2;
                }
                */
                // check for max and wrap around
                if (* (list->current->value) > list->current->max)
                {
                  * (list->current->value) = list->current->max;
                  //delta=1;
                }

                break;

              case   MENUTYPE_CHOICE:

                // check for start of choice list and wrap
                if (* (list->current->value) ==  0)
                {
                  * (list->current->value) = list->current->cnum - 1;
                }
                else
                {
                  // Menu with a list of choices
                  // value is used as an index
                  * (list->current->value) = * (list->current->value) - 1;
                }

                break;

              default:
                //RUNTIME_DEBUG2("INVALID KEY");
                break;
            } // menu_type

            break;

          case kButtonDown:
          case kButtonPgDown|kButtonDown:
            buttonState = button_process_done;

            // execute callback if defined
            //if (list->current->callback != NULL) {
            //  done = ((uint8_t (*)(void))list->current->callback)();
            //}

            switch (list->current->menu_type)
            {
              case   MENUTYPE_TEXT_FIELD:
                if (list->current->value == NULL)
                {
                  RUNTIME_ERROR("L->C->V NULL");
                }

                if (((char *) list->current->value) [edit_index] >= 32)
                {
                  ((char *) list->current->value) [edit_index]--;
                }

                break;

              case MENUTYPE_DB_ENTRY:
                menu_db_entry = (menu_db_t *) list->current->value;

                if (menu_db_entry == NULL)
                {
                  RUNTIME_ERROR("MT_DB_ENTRY NULL");
                  break;
                }

                // increase value
                menu_db_entry->obj_id = menu_db_entry->obj_id + accelerator;

                if (menu_db_entry->obj_id > db_get_size(menu_db_entry->db_id) - 1)
                {
                  menu_db_entry->obj_id = db_get_size(menu_db_entry->db_id) - 1;
                }

                break;

              case   MENUTYPE_EXIT:
                // An exit menu - we are done
                menu_retval = CONST_MENU_TERMINATE;
                buttonState = button_wait_exit;

                // execute callback if defined
                if (list->current->callback != NULL)
                {
                  lcd_clear();
                  menu_retval = ((uint8_t ( *)(int32_t)) list->current->callback)(list->current->callback_arg);
                }

                break;

              case   MENUTYPE_SUB:

                // selection using a sub menu
                if (list->current->list != NULL)
                {
                  // recursively enter menu run for sub menu
                  if (menu_run(list->current->list, NULL, idle_callback) == CONST_MENU_TERMINATE_ALL)
                  {
                    menu_retval = CONST_MENU_TERMINATE_ALL;
                    buttonState = button_wait_exit_all;
                  }
                }

                // execute callback if defined
                if (list->current->callback != NULL)
                {
                  menu_retval = ((uint8_t ( *)(int32_t)) list->current->callback)(list->current->callback_arg);
                }

                break;

              case   MENUTYPE_TIME:
                rtc_set(modulus_i(rtc_get() - accelerator2, CONST_SEC_PER_DAY));
                break;

              case   MENUTYPE_VALUE_FP:
                *((double *)list->current->value) -= (((double)accelerator2) * ((double)accelerator2) / 1e6);

                if (* ((double *)list->current->value) < list->current->min)
                {
                  * ((double *)list->current->value) = list->current->min;
                }

                break;

              case   MENUTYPE_VALUE_INT:
              case   MENUTYPE_VALUE_SHORT:
              case   MENUTYPE_VALUE_BYTE:
              case   MENUTYPE_DMS:
              case   MENUTYPE_HMS:
              case   MENUTYPE_DATE:
                // simple menu with a variable
                // decrease value
                * (list->current->value) -= accelerator2;

                /* DELETEME
                if (multiThreshold < 10) {
                  multiThreshold++;
                } else if (delta < 100000000) {
                  delta = delta*10;
                  multiThreshold = 2;
                }
                */
                // check for min and wrap around
                if (* (list->current->value) < list->current->min)
                {
                  * (list->current->value) = list->current->min;
                }

                break;

              case   MENUTYPE_CHOICE:

                // Menu with a list of choices
                // value is used as an index

                // check for end of choice list and wrap
                if (* (list->current->value) >= list->current->cnum - 1)
                {
                  * (list->current->value) = 0;
                }
                else
                {
                  * (list->current->value) = * (list->current->value) + 1;
                }

                break;

              default:
                //RUNTIME_DEBUG2("INVALID KEY");
                break;
            } // menu_type

            break;

          default:
            //RUNTIME_DEBUG2("MULTI KEY");
            buttonState = button_wait_press;
            break;
        } // processButton

        break;

      case button_wait_exit:
      case button_wait_exit_all:
        if (keypad_get() == kButtonNoKey)
        {
          lcd_clear();
          return (menu_retval);
        }

        break;

      default:
        // why are we here ??
        RUNTIME_ERROR("INVALID BSTATE");
        break;
    } // buttonState

    // delay
    delay_ms(10);

    if (idle_callback != NULL)
    {
      ((void ( *)(void)) idle_callback)();
    }
  } // while

  /*
    // wait for all keys to be released
    while (keypad_get() != kButtonNoKey) {
      delay_ms(25);
      if (idle_callback != NULL) {
        ((void (*)(void))idle_callback)();
      }
    }
  */
  lcd_clear();
  return (menu_retval);
}

#if 0
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*      Return 1 if a menu is currently active, 0 if otherwise                 */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
uint8_t menu_active(void)
{
  return (menu_retval == CONST_MENU_ACTIVE);
}

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*      Calculate unique magic for eeprom storage id'ing                      */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
uint32_t menu_checksum_get(menu_list_t *m)
{
  uint32_t seed, magic = 0, index = 1;
  uint8_t done = 0;

  if (m == NULL)
  {
    return 0x5a98fcaa;
  }

  //RUNTIME_DEBUG2("enter");
  menu_entry_t *p = m->first;

  while (!done)
  {
    // build up the magic
    seed = p->menu_type * index * 1;
    seed += p->min * index * 2;
    seed += p->max * index * 4;
    seed += p->cnum * index * 5;
    seed += (p->callback != NULL ? index : -index) * 6;
    seed += (p->list != NULL ? index : -index) * 7;
    seed += (index * index) * 8;
    seed += menu_checksum_get(p->list) * 9;
    magic = magic ^ seed;
    // next
    done = (p == m->last);
    p = p->next;
    index++;
  }

  //RUNTIME_DEBUG2("exit");
  return magic;
}
#endif

