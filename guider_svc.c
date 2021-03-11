/*
 * CFile1.c
 *
 * Created: 16/12/2012 6:43:10 PM
 *  Author: michal
 */
#include "project.h"

#define GUIDER_STACKSIZE        256         // Stack size in words
#define GUIDEUI_STACKSIZE       256         // Stack size in words
#define GUIDEKEY_STACKSIZE      512         // Stack size in words
#define PRIORITY_ASTRO_TASK       0         // Task priority

// task control flag
static uint8_t task_active = false;

// PID controller parameters
#define PID_MAX 2
static pidctl_t pid[PID_MAX] =
{
  // ra
  {
    .windup_guard      = 1,
    .gain              = &config.ra.pid
  },
  // dec
  {
    .windup_guard      = 1,
    .gain              = &config.dec.pid
  }
};

// configuration objects
config_t config;

static db_info_t db_info = {0, 0};
static coord_info_t pos_menu = {0, 0};

// holder for found nearest astro object
static uint32_t neighbors_param_list[2] = {0, 0};
static uint8_t neighbors_param_list_valid = false;

// position holders
static coord_info_t pos_current = {0, 0};
static coord_info_t pos_desired = {0, 0};
static coord_info_t pos_reference = {0, 0};

// time difference from the last calculation
static double time_delta = 0;

// dew heater
static heater_e heater_power = heater_off;

// current tracking mode & flags
static track_mode_e track_mode = track_sidereal;
static op_mode_e op_mode = mode_none;

// current battery state
static bat_state_e bat_state = bat_undefined;

// forward declarations
static int8_t config_save(config_t *c);
static uint8_t cb_align(int32_t arg);
static uint8_t cb_goto(int32_t arg);
static uint8_t cb_neighbors(int32_t arg);
//static uint8_t cb_config_invalidate(int32_t arg);
static void display_build();
static void update_coordinates();
static int8_t do_menu(uint8_t option, uint32_t param_list[]);
static void set_parameters(void);

// custom characters

const uint8_t custom_chars[33 * 8] __FLASH__ =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // level low
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // level high

  0x01, 0x02, 0x04, 0x0e, 0x1f, 0x1f, 0x1f, 0x0e, // bomb
  0x05, 0x02, 0x04, 0x0e, 0x1f, 0x1f, 0x1f, 0x0e, // bomb
  0x02, 0x03, 0x04, 0x0e, 0x1f, 0x1f, 0x1f, 0x0e, // bomb
  0x08, 0x06, 0x04, 0x0e, 0x1f, 0x1f, 0x1f, 0x0e, // bomb
  0x00, 0x0c, 0x04, 0x0e, 0x1f, 0x1f, 0x1f, 0x0e, // bomb
  0x00, 0x00, 0x04, 0x0e, 0x1f, 0x1f, 0x1f, 0x0e, // bomb
  0x00, 0x00, 0x00, 0x0e, 0x1f, 0x1f, 0x1f, 0x0e, // bomb
  0x11, 0x0a, 0x0a, 0x00, 0x15, 0x11, 0x1b, 0x0e, // bomb

  0x00, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x00, // stopped
  0x03, 0x0e, 0x1c, 0x1c, 0x1c, 0x1c, 0x0e, 0x03, // moon
  0x00, 0x00, 0x04, 0x15, 0x1b, 0x15, 0x04, 0x00, // star
  0x00, 0x0e, 0x1b, 0x11, 0x11, 0x1b, 0x0e, 0x00, // planet
  0x00, 0x0e, 0x1f, 0x1f, 0x1f, 0x1f, 0x0e, 0x00, // sun
  0x04, 0x0e, 0x1f, 0x0a, 0x0a, 0x1f, 0x0e, 0x04, // slew
  0x00, 0x0e, 0x15, 0x15, 0x17, 0x11, 0x0e, 0x00, // clock
  0x1b, 0x11, 0x00, 0x04, 0x00, 0x11, 0x1b, 0x00, // goto target

  0x04, 0x0e, 0x11, 0x00, 0x00, 0x11, 0x0e, 0x04, // up/down
  0x0e, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, // battery ok
  0x0e, 0x1f, 0x11, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, // battery ..
  0x0e, 0x1f, 0x11, 0x11, 0x1f, 0x1f, 0x1f, 0x1f, // battery ..
  0x0e, 0x1f, 0x11, 0x11, 0x11, 0x1f, 0x1f, 0x1f, // battery ..
  0x0e, 0x1f, 0x11, 0x11, 0x11, 0x11, 0x1f, 0x1f, // battery ..
  0x0e, 0x1f, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1f, // battery ..
  0x0e, 0x1f, 0x11, 0x19, 0x15, 0x13, 0x11, 0x1f, // battery critical

  0x04, 0x0e, 0x15, 0x04, 0x04, 0x00, 0x1f, 0x1f, // heater on
};

// hello
static void display_logo(void)
{
  uint8_t i = 0;
  char *p;
  lcd1602_write_cchar(0, 8 * 8, &custom_chars[CUST_CHR_LEVELa]);
  i = 0;
//p = "MICRO AUTOMATION";
  p = "    ROBOTICS    ";
  lcd1602_clear();
  lcd1602_write_str(0, 0, "\001\002\003\004\005\006x101\006\005\004\003\002\001");
  lcd1602_write_str(0, 1, "   \377\377\377\377\377\377\377\377\377\377   ");
  delay_ms(250);

  while (*p)
  {
    lcd1602_write_char(i, 1, *p);
    i ++;
    p ++;
    delay_ms(10);
  }

  delay_ms(500);
  lcd1602_clear();
}

// Set dew heater procedure
static uint8_t cb_heater(int32_t arg)
{
  return (CONST_MENU_TERMINATE_ALL); // recursively quit the menu
}

// Perform panorama procedure
static uint8_t cb_photo(int32_t arg)
{
  uint16_t n, i;
  double a, dps, nd;
  char b[LCD_WIDTH + 1];
  double angle;
  // crop dividers:
  // 0.5 - 0.52632
  // 1.0 - 1
  // 1.5 - 1.4835
  // 1.6 - 1.5789
  // 2.0 - 1.9708
#define NUM_PHOTO_PROFILES 17
  // field of view table - { hor, vert }
  float fov_table[NUM_PHOTO_PROFILES][2] =
  {
    {243.19, 154.05}, // 8mm fisheye - 132.1/112.6deg (1.6crop: 109.2/86.3deg); 1.6crop rectilinear equiv.: 162.13/102.7deg
    {121.9,   100.4}, // 10mm
    {112.6,    90.0}, // 12mm
    {100.4,    77.3}, // 15mm
    { 84.0,    61.9}, // 20mm
    { 73.7,    53.1}, // 24mm
    { 65.5,    46.4}, // 28mm
    { 54.4,    37.8}, // 35mm
    { 48.5,    33.4}, // 40mm
    { 39.6,    27.0}, // 50mm
    { 26.3,    17.7}, // 77mm
    { 23.9,    16.1}, // 85mm
    { 20.4,    13.7}, // 100mm
    { 15.2,    10.2}, // 135mm
    { 10.3,    6.90}, // 200mm
    { 6.9,     4.60}, // 300mm
    { 5.15,    3.44}  // 400mm
  };
#define ANIMATION_SIZE 22
  uint8_t animation_table[ANIMATION_SIZE] = {0, 1, 2, 1, 2, 3, 2, 3, 4, 3, 4, 5, 4, 5, 6, 5, 6, 6, 7, 6, 7, 7};
  //stepperhw_init();
  lcd_clear();
  nd = config.photo.numdegrees;
  dps = config.photo.degpershot;
  nd = 360;

  // angles of advance. typically 1/2 of narrow angle of view of the corresponding lens
  // default profile
  if (config.photo.profile == 0)
  {
    nd = config.photo.numdegrees;
    dps = config.photo.degpershot;
  }
  else
  {
    // calculate using the fov table
    nd = 360;

    switch (config.photo.orientation)
    {
      case panorama_orientation_horizontal:
        // horizontal - half of crop-corrected hfov
        dps = fov_table[(config.photo.profile) - 1][0];
        break;

      case panorama_orientation_vertical:
        // vertical  - half of crop-corrected vfov
        dps = fov_table[(config.photo.profile) - 1][1];
        break;

      default:
        RUNTIME_ERROR("photo_orientation");
        break;
    }

    // finish the calculation
    dps = ((dps * 10) / (2 * config.photo.cropfactor));
  }

  n = 1;

  for (a = 0; a < nd; a += dps)
  {
    snprintf(b, sizeof(b), "PANORAMA %3i/%-3i", n, (int)(nd / dps) + 1);
    b[LCD_WIDTH] = 0;
    lcd_write_str(0, 0, b);
    snprintf(b, sizeof(b), "%3.1f%c", a, CHR_DEGREE);
    b[LCD_WIDTH] = 0;
    lcd_write_str(10, 1, b);
    // stabilize
    lcd_write_str(1, 1, " -WAIT-");

    for (i = 0; i < ANIMATION_SIZE; i++)
    {
      lcd_custom_char(CHR_CUSTOM_1, CUST_CHR_BOMBa + animation_table[i], custom_chars);
      lcd_write_char(0, 1, CHR_CUSTOM_1);
      delay_ms(100);
    }

    // take the shot
    // TODO
    lcd_custom_char(CHR_CUSTOM_1, CUST_CHR_CLOCK, custom_chars);

    for (i = config.photo.waitpershot - 1 ; i > 0 ; i--)
    {
      snprintf(b, sizeof(b), "%c % 3i.%-1is", CHR_CUSTOM_1, (int)((i - 1) / 10), (int)((i - 1) % 10));
      b[LCD_WIDTH - 1] = 0;
      lcd_write_str(0, 1, b);
      delay_ms(100);
    }

    n++;
    // simulate motor moving
    lcd_write_char(0, 1, CHR_RIGHTARROW);
    lcd_write_str(1, 1, " ......");
    angle = dps;
    time_delta = CONV_SEC_TO_ANGLE(rtc_delta_get());
    stepperhw_move(0, angle, time_delta, config.ra.max_speed, config.ra.steps_per_deg);

    if (a >= nd - dps)
    {
      continue;
    }

    angle = -1;
    i = 0;

    while ((angle != 0) && i < 5)
    {
      lcd_write_char(2 + i, 1, '>');
      delay_ms(100);
      angle = 0;
      stepperhw_move(0, angle, time_delta, config.ra.max_speed, config.ra.steps_per_deg);
      lcd_write_char(2 + i, 1, '.');
      i = (i + 1) % 6;
      angle = 1; /*FIXME*/
    }
  }

  lcd_write_char(0, 1, CHR_LEFTARROW);
  lcd_write_str(1, 1, " <<<<<<");
  delay_ms(1000);
  lcd_clear();
  return (CONST_MENU_TERMINATE_ALL); // recursively quit the menu
}

// Perform alignment procedure
static uint8_t cb_align(int32_t arg)
{
  db_entry_t *db_entry = NULL;

  // sanity check
  if (db_info.db_id >= db_get_num())
  {
    RUNTIME_ERROR("INVALID DBID");
  }

  if (db_info.obj_id >= db_get_size(db_info.db_id))
  {
    RUNTIME_ERROR("INVALID DBOBJID");
  }

  // TODO...
  db_entry = db_get_object(db_info.db_id, db_info.obj_id);

  if (db_entry == NULL)
  {
    RUNTIME_ERROR("NULL DB ENTRY");
  }

  // set reference mark
  guider_cmd(cmd_align, CONV_SEC_TO_ANGLE(db_entry->ra), CONV_SEC_TO_ANGLE(db_entry->dec));
  //pos_reference.ra = modulus_d(CONV_SEC_TO_ANGLE(db_entry->ra) - pos_current.ra, 360);
  //pos_reference.dec = modulus_d(CONV_ARCSEC_TO_ANGLE(db_entry->dec) - pos_current.dec, 360);
  DEBUG_PRINT("Reference set: ra ");
  DEBUG_PRINT_FLOAT(pos_reference.ra);
  DEBUG_PRINT(" dec ");
  DEBUG_PRINT_FLOAT(pos_reference.dec);
  DEBUG_PRINT("\n");
  //delay_ms(1000);
  return (CONST_MENU_TERMINATE_ALL); // recursively quit the menu
}

// Perform goto procedure
static uint8_t cb_goto(int32_t arg)
{
  db_entry_t *db_entry = NULL;

  if ((arg == -1)
      || ((arg >= 0) && (arg < db_get_num())))   // db-goto
  {
    if (arg != -1)
    {
      db_info.db_id = arg;
    }

    // sanity check
    if (db_info.db_id >= db_get_num())
    {
      RUNTIME_ERROR("INVALID DBID");
    }

    if (db_info.obj_id >= db_get_size(db_info.db_id))
    {
      RUNTIME_ERROR("INVALID DBOBJID");
    }

    db_entry = db_get_object(db_info.db_id, db_info.obj_id);

    if (db_entry == NULL)
    {
      RUNTIME_ERROR("NULL DB ENTRY");
    }

    guider_cmd(cmd_goto, CONV_SEC_TO_ANGLE(db_entry->ra), CONV_SEC_TO_ANGLE(db_entry->dec));
    //pos_desired.ra = modulus_d(CONV_SEC_TO_ANGLE(db_entry->ra) + pos_reference.ra, 360);
    //pos_desired.dec = modulus_d(CONV_ARCSEC_TO_ANGLE(db_entry->dec) + pos_reference.dec, 360);
  }
  else if (arg == -2)       // coordinate-goto
  {
    guider_cmd(cmd_goto, pos_menu.ra, pos_menu.dec);
    //pos_desired.ra = modulus_d(CONV_SEC_TO_ANGLE(pos_menu.ra) + pos_reference.ra, 360);
    //pos_desired.dec = modulus_d(CONV_ARCSEC_TO_ANGLE(pos_menu.dec) + pos_reference.dec, 360);
  }
  else
  {
    RUNTIME_DEBUG_VA("ARG %i", (int) arg);
    RUNTIME_ERROR("INVALID ARG");
  }

//RUNTIME_DEBUG_VA("dec %lu", dec);
  return (CONST_MENU_TERMINATE_ALL); // recursively quit the menu
}

// Perform find neighbors procedure
static uint8_t cb_neighbors(int32_t arg)
{
  uint8_t db_id, closest_db_id, found;
  uint16_t obj_id, closest_obj_id, sum_all_obj, current_obj, db_size;
  double closest_dist, dist;
  db_entry_t *db_entry;
  //menu_db_t *menu_db_entry;
  char b[LCD_WIDTH + 1];
  int8_t percentage, percentage_shadow;
  lcd_clear();
  lcd_write_str(0, 0, "SEARCHING DB");
  sum_all_obj = 0;

  for (db_id = 0; db_id < db_get_num(); db_id++)
  {
    if ((config.location.whereami_db_num == 0)
        || (db_id == config.location.whereami_db_num - 1))
    {
      sum_all_obj += db_get_size(db_id);
    }
  }

  coord_info_t pos;
  pos.ra = modulus_d(CONV_SEC_TO_ANGLE(pos_current.ra) + pos_reference.ra, 360);
  pos.dec = modulus_d(CONV_SEC_TO_ANGLE(pos_current.dec) + pos_reference.dec, 360);
  current_obj = 1;
  closest_dist = (uint32_t) - 1;
  found = 0;
  percentage_shadow = -1;

  for (db_id = 0; db_id < db_get_num(); db_id++)
  {
    if ((config.location.whereami_db_num == 0)
        || (db_id == config.location.whereami_db_num - 1))
    {
      db_size = db_get_size(db_id);

      for (obj_id = 0; obj_id < db_size; obj_id++)
      {
        coord_info_t tmp;
        db_entry = db_get_object(db_id, obj_id);
        // check closes distance, convert RA to angle value like DEC
        tmp.ra = modulus_d(CONV_SEC_TO_ANGLE(db_entry->ra) - pos.ra, 360);
        tmp.dec = modulus_d(CONV_SEC_TO_ANGLE(db_entry->dec) - pos.dec, 360);
        dist = sqrt((tmp.ra * tmp.ra) + (tmp.dec * tmp.dec));

        //DEBUG_PRINT("NEIGH: db %i, obj %i, sum %i, dist %i\n", db_id, obj_id, sum_all_obj, (uint32_t)dist);
        //RUNTIME_DEBUG_VA("d=%lu", dist);

        if (dist < closest_dist)
        {
          closest_dist = dist;
          closest_db_id = db_id;
          closest_obj_id = obj_id;
          found = 1;
        }

        // update display
        percentage = ((uint32_t) current_obj * 100) / sum_all_obj;
        current_obj++;

        if ((percentage >> 2) != (percentage_shadow >> 2))
        {
          sprintf(b, "%-10s  %3i%%", db_get_name(db_id), percentage);
          lcd_write_str(0, 1, b);
          sprintf(b, "%i", db_entry->valid);
          percentage_shadow = percentage;
        }
      }
    }
  }

  lcd_clear();

  if (found == 0)
  {
    runtime_msg("NOT FOUND");
  }
  else
  {
    DEBUG_PRINT("NEIGHBOUR SEARCH RESULT: db %i, obj %i, dist %i\n", closest_db_id, closest_obj_id, (uint32_t)closest_dist);
    db_entry = db_get_object(closest_db_id, closest_obj_id);
    snprintf(b, sizeof(b), "Found: %s", db_entry->name);
    b[sizeof(b) - 1] = 0;
    runtime_msg(b);
    neighbors_param_list[0] = closest_db_id;
    neighbors_param_list[1] = closest_obj_id;
    neighbors_param_list_valid = true;
  }

  lcd_clear();
  return (CONST_MENU_TERMINATE_ALL);
}

static void set_config_defaults(config_t *config)
{
  memset((void *)config, 0, sizeof(config_t));
  config->ra.dir = 0;
  config->ra.max_speed = CONST_DEFAULT_STEPS_PER_SEC;
  config->ra.pwm_drive = 50;
  config->ra.steps_per_deg = CONST_STEPS_PER_MOUNT_DEG;
  config->ra.pid.proportional = +0.010;
  config->ra.pid.integral = +0.000;
  config->ra.pid.derivative = +0.000;
  config->dec.dir = 0;
  config->dec.max_speed = CONST_DEFAULT_STEPS_PER_SEC;
  config->dec.pwm_drive = 50;
  config->dec.steps_per_deg = CONST_STEPS_PER_MOUNT_DEG;
  config->dec.pid.proportional = +0.010;
  config->dec.pid.integral = +0.000;
  config->dec.pid.derivative = +0.000;
  config->focus.dir = 0;
  config->focus.max_speed = CONST_DEFAULT_STEPS_PER_SEC;
  config->focus.pwm_drive = 50;
  config->focus.steps_per_deg = CONST_STEPS_PER_MOUNT_DEG;
  config->focus.pid.proportional = +0.100;
  config->focus.pid.integral = +0.000;
  config->focus.pid.derivative = +0.000;
  config->aux.dir = 0;
  config->aux.max_speed = CONST_DEFAULT_STEPS_PER_SEC;
  config->aux.pwm_drive = 50;
  config->aux.steps_per_deg = CONST_STEPS_PER_MOUNT_DEG;
  config->aux.pid.proportional = +0.100;
  config->aux.pid.integral = +0.000;
  config->aux.pid.derivative = +0.000;
  config->mount.init_track_mode = track_sidereal;
  config->mount.type = mount_geq;
}

// Reading configuration from external storage
static int8_t config_load(config_t *c)
{
  eeprom_read_block(c, 0, sizeof(config_t));

  if (c->magick != CONST_PROJ_CFG_VERSION) //menu_magic_get(main_menu))
  {
    set_config_defaults(c);
    runtime_msg("DEFAULT CONFIG");
    DEBUG_PRINT("Ignoring eeprom config due to magick mismatch. Using default config.\n");
  }
  else
  {
    DEBUG_PRINT("Read config from eeprom\n");
  }

  return 0;
}

#if 0
// Invalidate configuration on external storage
static uint8_t cb_config_invalidate(int32_t arg)
{
  config.magick = ~CONST_PROJ_CFG_VERSION;
  return config_save(&config);
}
#endif

// Write configuration to external storage
static int8_t config_save(config_t *c)
{
  if (c == NULL)
  {
    RUNTIME_ERROR("NULL CONFIG");
  }

  eeprom_write_block(c, 0, sizeof(config_t));
  return 0;
}

// Main menu code
static int8_t do_menu(uint8_t option, uint32_t param_list[])
{
  menu_entry_t *m, *c, *n;
  uint8_t index;
  db_list_t *db;
  menu_entry_t *goto_db_menu[db_list_end] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
  //char test_text_field[5] = "edit1";
  static menu_list_t *main_menu = NULL;
  void (*idle_callback)() = NULL;
  menu_entry_t *init = NULL;
  op_mode_e previous_op_mode = op_mode;
  op_mode = mode_menu;
  lcd_acquire();
  LCDBacklight = 1;
  menu_init();
  main_menu = menu_list_alloc();

  // Start menus
  switch (option)
  {
    case menu_photo:
      idle_callback = NULL;
      c = menuItemChoiceAlloc(main_menu, "Pano Select", config.photo.profile);
      menu_choice_alloc(c, "Custom");
      menu_choice_alloc(c, "8mm / 360 FE");
      menu_choice_alloc(c, "10mm / 360");
      menu_choice_alloc(c, "12mm / 360");
      menu_choice_alloc(c, "15mm / 360");
      menu_choice_alloc(c, "20mm / 360");
      menu_choice_alloc(c, "24mm / 360");
      menu_choice_alloc(c, "28mm / 360");
      menu_choice_alloc(c, "35mm / 360");
      menu_choice_alloc(c, "40mm / 360");
      menu_choice_alloc(c, "50mm / 360");
      menu_choice_alloc(c, "77mm / 360");
      menu_choice_alloc(c, "85mm / 360");
      menu_choice_alloc(c, "100mm / 360");
      menu_choice_alloc(c, "135mm / 360");
      menu_choice_alloc(c, "200mm / 360");
      menu_choice_alloc(c, "300mm / 360");
      menu_choice_alloc(c, "400mm / 360");
      c = menuItemChoiceAlloc(main_menu, "Orientation", config.photo.orientation);
      menu_choice_alloc(c, "Vertical");
      menu_choice_alloc(c, "Horizontal");
      menuItemExitAllocCb(main_menu, "Execute", cb_photo, 0);
      menuItemValueIntAlloc(main_menu, "Degrees/shot", config.photo.degpershot, 1, 10, 999);
      menuItemValueIntAlloc(main_menu, "Total degrees", config.photo.numdegrees, 1, 360, 999);
      menuItemValueIntAlloc(main_menu, "Delay *100ms", config.photo.waitpershot, 1, 50, 999);
      menuItemValueIntAlloc(main_menu, "Crop Factor /10", config.photo.cropfactor, 5, 15, 20);
      menuItemValueIntAlloc(main_menu, "Stepper (stp/dg)", config.photo.motor_stepsdeg, 1, 1000, 99999);
      break;

    case menu_dew_heater:
      idle_callback = NULL;
      // dew heater
      c = menuItemChoiceAllocCb(main_menu, "Dew Heater", heater_power, cb_heater, 0);
      menu_choice_alloc(c, "OFF");
      menu_choice_alloc(c, "25%");
      menu_choice_alloc(c, "50%");
      menu_choice_alloc(c, "75%");
      menu_choice_alloc(c, "ON");
      break;

    case menu_goto:
      idle_callback = NULL;
      // goto
      m = menuItemSubmenuAlloc(main_menu, "GoTo Object");
      m->list = menu_list_alloc();

      for (index = 0; index < db_list_end; index++)
      {
        db = db_get(index);

        if (db != NULL)
        {
          goto_db_menu[index] = menuItemDBEntryAllocCb(m->list, db->name, db_info, index, cb_goto, index);
          //menuItemExitAllocCb(m->list, "Execute", cb_goto, -1);
        }
      }

      n = menuItemSubmenuAlloc(m->list, "Coordinates");
      n->list = menu_list_alloc();
      menuItemHMSAlloc(n->list, "R/A", pos_menu.ra);
      menuItemDMSAlloc(n->list, "DEC", pos_menu.dec);
      menuItemExitAllocCb(n->list, "Execute", cb_goto, -2);
      //menuItemExitAlloc(n->list, "Exit SubMenu");

      //menuItemExitAlloc(m->list, "Exit");

      if (param_list != NULL) // shortcut to a specific object
      {
        menu_db_t *menu_db_entry;
        menu_db_entry = (menu_db_t *) goto_db_menu[param_list[0]]->value;
        menu_db_entry->db_id = param_list[0];
        menu_db_entry->obj_id = param_list[1];
        init = (menu_entry_t *) goto_db_menu[param_list[0]];
      }

      break;

    case menu_neighbour:
      idle_callback = NULL;
      // what's nearby
      m = menuItemSubmenuAlloc(main_menu, "Nearby Objects");
      m->list = menu_list_alloc();
      c = menuItemChoiceAllocCb(m->list, "DB Select", config.location.whereami_db_num, cb_neighbors, 0);
      menu_choice_alloc(c, "All");

      for (index = 0; index < db_list_end; index++)
      {
        db = db_get(index);

        if (db != NULL)
        {
          menu_choice_alloc(c, db->name);
        }
      }

      //menuItemExitAllocCb(m->list, "Search DB", cb_neighbors, 0);
      //menuItemExitAlloc(m->list, "Exit SubMenu");
      break;

    case menu_setup:
      idle_callback = NULL;
      // align
      m = menuItemSubmenuAlloc(main_menu, "Align to Obj.");
      m->list = menu_list_alloc();

      for (index = 0; index < db_list_end; index++)
      {
        db = db_get(index);

        if (db != NULL)
        {
          menuItemDBEntryAllocCb(m->list, db->name, db_info, index, cb_align, index);
          //menuItemExitAllocCb(m->list, "Execute", cb_align, -1);
        }
      }

      menuItemExitAllocCb(m->list, "Align Reset", cb_align, 0);
      menuItemExitAlloc(m->list, "Exit SubMenu");
      // time/location setup
      m = menuItemSubmenuAlloc(main_menu, "Time/Location");
      m->list = menu_list_alloc();
      menuItemClockAlloc(m->list, "Set Time", config.time.time);
      menuItemDateAlloc(m->list, "Set Date", config.time.date);
      //menuItemHMSAlloc(m->list, "Sample HSM", config.latitude);
      menuItemDMSAlloc(m->list, "Latitude", config.location.latitude);
      menuItemDMSAlloc(m->list, "Longitude", config.location.longitude);
      //menuItemValueIntAlloc(m->list, "Latitude: Deg", config.latitude, -90, 90);
      menuItemExitAlloc(m->list, "Exit SubMenu");
      // system config
      m = menuItemSubmenuAlloc(main_menu, "System Config");
      m->list = menu_list_alloc();
      c = menuItemChoiceAlloc(m->list, "Startup Mode", config.mount.init_track_mode);
      menu_choice_alloc(c, "Idle");
      menu_choice_alloc(c, "Sidereal");
      c = menuItemChoiceAlloc(m->list, "Mount Type", config.mount.type);
      menu_choice_alloc(c, "German Equat.");
      menu_choice_alloc(c, "Alt-Azimuth");
      c = menuItemChoiceAlloc(m->list, "RA Limit", config.mount.limit_ra);
      menu_choice_alloc(c, "Unlimited");
      menu_choice_alloc(c, "360deg");
      c = menuItemChoiceAlloc(m->list, "DEC Limit", config.mount.limit_dec);
      menu_choice_alloc(c, "-90...+90 deg");
      menu_choice_alloc(c, " -2...+90 deg");
      c = menuItemChoiceAlloc(m->list, "RA Direction", config.ra.dir);
      menu_choice_alloc(c, "Forward");
      menu_choice_alloc(c, "Backward");
      menuItemValueIntAlloc(m->list, "R/A (stp/dg)", config.ra.steps_per_deg, 1, CONST_STEPS_PER_MOUNT_DEG, 10000);
      menuItemValueIntAlloc(m->list, "R/A Max (p/s)", config.ra.max_speed, 100, 900, CONST_DEFAULT_STEPS_PER_SEC);
      menuItemValueIntAlloc(m->list, "R/A PWM (%)", config.ra.pwm_drive, 1, 10, 100);
      menuItemValueFPAlloc(m->list, "R/A Pid gain", config.ra.pid.proportional, 0, 0.01, +1);
      menuItemValueFPAlloc(m->list, "R/A pId gain", config.ra.pid.integral, 0, 0.000, +1);
      menuItemValueFPAlloc(m->list, "R/A piD gain", config.ra.pid.derivative, 0, 0.000, +1);
      c = menuItemChoiceAlloc(m->list, "DEC Direction", config.dec.dir);
      menu_choice_alloc(c, "Forward");
      menu_choice_alloc(c, "Backward");
      menuItemValueIntAlloc(m->list, "DEC (stp/dg)", config.dec.steps_per_deg, 1, CONST_STEPS_PER_MOUNT_DEG, 10000);
      menuItemValueIntAlloc(m->list, "DEC Max (p/s)", config.dec.max_speed, 100, 900, CONST_DEFAULT_STEPS_PER_SEC);
      menuItemValueIntAlloc(m->list, "DEC PWM (%)", config.dec.pwm_drive, 1, 10, 100);
      menuItemValueFPAlloc(m->list, "DEC Pid gain", config.dec.pid.proportional, 0, 0.01, +1);
      menuItemValueFPAlloc(m->list, "DEC pId gain", config.dec.pid.integral, 0, 0.000, +1);
      menuItemValueFPAlloc(m->list, "DEC piD gain", config.dec.pid.derivative, 0, 0.000, +1);
      c = menuItemChoiceAlloc(m->list, "FOC Direction", config.focus.dir);
      menu_choice_alloc(c, "Forward");
      menu_choice_alloc(c, "Backward");
      menuItemValueIntAlloc(m->list, "FOC (stp/dg)", config.focus.steps_per_deg, 1, CONST_STEPS_PER_MOUNT_DEG, 10000);
      menuItemValueIntAlloc(m->list, "FOC Max (p/s)", config.focus.max_speed, 100, 100, CONST_DEFAULT_STEPS_PER_SEC);
      menuItemValueIntAlloc(m->list, "FOC PWM (%)", config.focus.pwm_drive, 1, 10, 100);
      menuItemValueFPAlloc(m->list, "FOC Pid gain", config.focus.pid.proportional, 0, 0.10, +1);
      menuItemValueFPAlloc(m->list, "FOC pId gain", config.focus.pid.integral, 0, 0.000, +1);
      menuItemValueFPAlloc(m->list, "FOC piD gain", config.focus.pid.derivative, 0, 0.000, +1);
      c = menuItemChoiceAlloc(m->list, "AUX Direction", config.aux.dir);
      menu_choice_alloc(c, "Forward");
      menu_choice_alloc(c, "Backward");
      menuItemValueIntAlloc(m->list, "AUX (stp/dg)", config.aux.steps_per_deg, 1, CONST_STEPS_PER_MOUNT_DEG, 10000);
      menuItemValueIntAlloc(m->list, "AUX Max (p/s)", config.aux.max_speed, 100, 900, CONST_DEFAULT_STEPS_PER_SEC);
      menuItemValueIntAlloc(m->list, "AUX PWM (%)", config.aux.pwm_drive, 1, 10, 100);
      menuItemValueFPAlloc(m->list, "AUX Pid gain", config.aux.pid.proportional, 0, 0.10, +1);
      menuItemValueFPAlloc(m->list, "AUX pId gain", config.aux.pid.integral, 0, 0.000, +1);
      menuItemValueFPAlloc(m->list, "AUX piD gain", config.aux.pid.derivative, 0, 0.000, +1);
//      menuItemValueIntAlloc(m->list, "HEATER1 Cycle", config.heater1, 1, 1, heater_max);
//      menuItemValueIntAlloc(m->list, "HEATER2 Cycle", config.heater2, 1, 1, heater_max);
      menuItemExitAlloc(m->list, "Exit SubMenu");
#if 0
      // debug features
      m = menuItemSubmenuAlloc(main_menu, "Test Features");
      m->list = menu_list_alloc();
      menuItemExitAllocCb(m->list, "Init EEPROM", cb_config_invalidate, 0);
      menuItemExitAllocCb(m->list, "Display Test", (uint8_t ( *)()) lcd_test, 0);
      menuItemExitAllocCb(m->list, "Keyboard Test", (uint8_t ( *)()) keypad_test, 0);
      menuItemExitAllocCb(m->list, "ADC Test", (uint8_t ( *)()) adc_test, 0);
      menuItemExitAllocCb(m->list, "Joystick Test", (uint8_t ( *)()) joystick_test, 0);
      menuItemExitAllocCb(m->list, "Stepper Test", (uint8_t ( *)()) stepperhw_test, 0);
      //menuItemValueIntAlloc(m->list, "Scrn Refr (ms)", config.scr_refresh,10,5000);
#endif
      menuItemExitAlloc(m->list, "Exit SubMenu");
      //menuItemTextFielAlloc(m->list, "sample text", test_text_field, sizeof(test_text_field)-1);
//    menuItemExitAlloc(main_menu, "Exit Menu");
      break;

    default:
      RUNTIME_ERROR("INVALID MENUOP")
      break;
  }

  int8_t rc = menu_run(main_menu, init, idle_callback);
  // propagate changed settings
  //set_parameters();
  // store configuration
  config.magick = CONST_PROJ_CFG_VERSION; //menu_magic_get(main_menu);
  config_save(&config);

  if (op_mode == mode_menu) // restore original mode if a new one has not been set
  {
    op_mode = previous_op_mode;
  }

  lcd_release();
  return rc;
}

void set_pid(float p, float i, float d, float w, uint32_t index)
{
  if (index >= PID_MAX)
  {
    DEBUG_PRINT("PID index %i out of bounds, max %i\n", index, PID_MAX - 1);
  }

  DEBUG_PRINT("Old PID %i:", index);
  DEBUG_PRINT("\n  proportional ");
  DEBUG_PRINT_FLOAT(pid[index].gain->proportional);
  DEBUG_PRINT("\n  integral     ");
  DEBUG_PRINT_FLOAT(pid[index].gain->integral);
  DEBUG_PRINT("\n  derivative   ");
  DEBUG_PRINT_FLOAT(pid[index].gain->derivative);
  DEBUG_PRINT("\n  windup_guard ");
  DEBUG_PRINT_FLOAT(pid[index].windup_guard);
  DEBUG_PRINT("\n");
  pid[index].gain->proportional = p;
  pid[index].gain->integral = i;
  pid[index].gain->derivative = d;
  pid[index].windup_guard = w;
  DEBUG_PRINT("New PI D%i:", index);
  DEBUG_PRINT("\n  proportional ");
  DEBUG_PRINT_FLOAT(pid[index].gain->proportional);
  DEBUG_PRINT("\n  integral     ");
  DEBUG_PRINT_FLOAT(pid[index].gain->integral);
  DEBUG_PRINT("\n  derivative   ");
  DEBUG_PRINT_FLOAT(pid[index].gain->derivative);
  DEBUG_PRINT("\n  windup_guard ");
  DEBUG_PRINT_FLOAT(pid[index].windup_guard);
  DEBUG_PRINT("\n");
  pid_zeroize(&pid[index]);
}

static void update_coordinates()
{
  coord_info_t angle_error;
  coord_info_t angle_delta;
  int8_t j0, j1;
  time_delta = rtc_delta_get();

  // auto-update ra in sidereal mode
  if (track_mode == track_sidereal)
  {
    pos_desired.ra += modulus_d(CONV_SEC_TO_ANGLE(time_delta), 360);
  }

 // update reference ra (sky's always moving)
  pos_reference.ra += modulus_d(CONV_SEC_TO_ANGLE(time_delta), 360);

  // ignore joystick in menu or aux mode
  if ((op_mode == mode_menu) || (op_mode == mode_aux))
  {
    j0 = j1 = 0;
  }
  else
  {
    j0 = joystick_get(0);
    j1 = joystick_get(1);
  }

   // xxx FIXME add limiters

#if 0
  // ra flip:
  // desired: ra = -40, dec = 10 is the same as ra=40, dec=190
  // dec = modulus_d(
  // FIXME adjust the error instead
  if ((pos_desired_tmp.ra < -2) && enabled)
  {
    pos_desired_tmp.ra = -pos_desired_tmp.ra;
    pos_desired_tmp.dec = modulus_d(pos_desired_tmp.dec + 180, 360);
  }

  // ra limit -90...+90 (stop if out of bounds)
  // FIXME adjust the error instead
  pos_desired_tmp.ra = limit_d(pos_desired_tmp.ra, -90, +90);

  // dec limit
  // calculate dec error differently when dec limit enabled so that 359 -> 1 deg results in 358 deg error (and not 2 deg)
  if (enabled)
  {
    angle_error.dec = two_compl_d(pos_desired.dec - pos_current.dec, 360); // -360 ... +360 result
  }
  else
  {
    angle_error.dec = two_compl_d(pos_desired.dec - pos_current.dec, 180); // -180 ... +180 result
  }
#endif

  // calculate required motor movement angles
  // send command to the stepper controllers
  // leave ra as 0..360 and dec as -2..90
  // update actual angle
  angle_error.ra = two_compl_d(pos_desired.ra
                               - pos_current.ra, 180); // -180 ... +180 (auto-chooses direction)

  // update with joystick vector
  angle_error.ra += ((float)j0 * (float)config.ra.max_speed) / ((float)config.ra.steps_per_deg * 100);

  pid_update(&pid[0], angle_error.ra, time_delta);  // ra pid controller
  
  angle_delta.ra = stepperhw_move(0, config.ra.dir == dir_forward ? +pid[0].control : -pid[0].control, // j0,
                                  time_delta, config.ra.max_speed, config.ra.steps_per_deg);
  pos_current.ra = modulus_d(pos_current.ra + angle_delta.ra, 360);
#if 0
  DEBUG_PRINT("MWD: ");
  DEBUG_PRINT_FLOAT(pos_current.ra);
  DEBUG_PRINT(" + ");
  DEBUG_PRINT_FLOAT(angle_delta.ra);
  DEBUG_PRINT(" = ");
  DEBUG_PRINT_FLOAT(pos_current.ra);
  DEBUG_PRINT("\n");
#endif
  angle_error.dec = two_compl_d(pos_desired.dec - pos_current.dec, 180);

  // update with joystick vector
  angle_error.ra += ((float)j0 * (float)config.dec.max_speed) / ((float)config.dec.steps_per_deg * 100);

  pid_update(&pid[1], angle_error.dec, time_delta);  // ra pid controller

  angle_delta.dec = stepperhw_move(1, config.dec.dir == dir_forward ? +pid[1].control : -pid[1].control, // j1,
                                   time_delta, config.dec.max_speed, config.dec.steps_per_deg);
  //pos_current.dec = two_compl_d(pos_current.dec + dec_angle_delta, 180);
  pos_current.dec = limit_d(pos_current.dec + angle_delta.dec, -2, 180);
  // debug info
  {
    static uint32_t i = 0;

    if (i == 1000)
    {
      DEBUG_PRINT("*** PID0:");
      DEBUG_PRINT(" dt ");
      DEBUG_PRINT_FLOAT(time_delta);
      DEBUG_PRINT(", control ");
      DEBUG_PRINT_FLOAT((float) pid[0].control);
      DEBUG_PRINT(", target_angle ");
      DEBUG_PRINT_FLOAT(pos_desired.ra);
      DEBUG_PRINT(", real_angle ");
      DEBUG_PRINT_FLOAT(pos_current.ra);
      DEBUG_PRINT(", error ");
      DEBUG_PRINT_FLOAT(angle_error.ra);
      DEBUG_PRINT(" ***\n");
      DEBUG_PRINT("*** PID1:");
      DEBUG_PRINT(" control ");
      DEBUG_PRINT_FLOAT((float) pid[1].control);
      DEBUG_PRINT(", target_angle ");
      DEBUG_PRINT_FLOAT(pos_desired.dec);
      DEBUG_PRINT(", real_angle ");
      DEBUG_PRINT_FLOAT(pos_current.dec);
      DEBUG_PRINT(", error ");
      DEBUG_PRINT_FLOAT(angle_error.dec);
      DEBUG_PRINT(" ***\n");
      i = 0;
    }
    else
    {
      i++;
    }
  }

  // op_mode update
  if ((j0 != 0) || (j1 != 0))
  {
    op_mode = mode_slew;
    pos_desired.ra = pos_current.ra;
    pos_desired.dec = pos_current.dec;
  }
  else if (op_mode == mode_goto)   // end of goto mode
  {
    // stop goto if target reached or key pressed
    if (((ABS(angle_error.ra) < CONST_GOTO_STOP_THRESHOLD) && (ABS(angle_error.dec) < CONST_GOTO_STOP_THRESHOLD))
        || (j0 != 0) || (j1 != 0))
    {
      DEBUG_PRINT("GOTO END:");
      DEBUG_PRINT(" ra ");
      DEBUG_PRINT_FLOAT(pos_current.ra);
      DEBUG_PRINT(", de ");
      DEBUG_PRINT_FLOAT(pos_current.dec);
      DEBUG_PRINT(", ra err ");
      DEBUG_PRINT_FLOAT(angle_error.ra);
      DEBUG_PRINT(", de err ");
      DEBUG_PRINT_FLOAT(angle_error.dec);
      DEBUG_PRINT("\n");
      pos_desired.ra = pos_current.ra;
      pos_desired.dec = pos_current.dec;
      op_mode = mode_none;
    }
  }
  else if (op_mode == mode_slew)   // end slew mode
  {
    op_mode = mode_none;
  }

  // check for system errors
  stepperhw_check_status();
}

static void update_display()
{
  double alt = 0, az = 0;
  static uint8_t blink_phase = 0;
  char b[LCD_WIDTH + 1], b2[5], c;
  int32_t tmp;
  op_mode_e previous_op_mode = op_mode;

  while (task_active == true)
  {
    lcd_acquire();

    // detect op_mode change, reset blink_phase */
    if (previous_op_mode != op_mode)
    {
      blink_phase = 0;
      previous_op_mode = op_mode;
    }

    /* battery sense */
    bat_state = (255 - adc_get(6)) / (256 / 8);

    switch (bat_state)
    {
      case bat_8:
      case bat_7:
      case bat_6:
      case bat_5:
      case bat_4:
      case bat_3:
      case bat_2:
        lcd_custom_char(CHR_CUSTOM_0, CUST_CHR_BATa + bat_state, custom_chars);
        //lcd_setbattery_char(CHARSET_BATTERY, bat_state);
        c = CHR_CUSTOM_0;
        break;

      case bat_1:
        lcd_custom_char(CHR_CUSTOM_0, CUST_CHR_BATg, custom_chars);
        //lcd_setbattery_char(CHARSET_BATTERY, bat_2);
        c = (blink_phase & 4 ? CHR_BLANK : CHR_CUSTOM_0);
        break;

      default:
        c = CHR_BLANK;
        break;
    }

    lcd_write_char(15, 1, c);

    switch (op_mode)
    {
#if 0

      case track_lunar:
        lcd_setmode_char(CHRSET_MODE, 1);
        c = (counter & 4 ? CHRSET_MODE : CHR_BLANK);
        break;

      case track_planetary:
        lcd_setmode_char(CHRSET_MODE, 3);
        c = (counter & 4 ? CHRSET_MODE : CHR_BLANK);
        break;

      case track_solar:
        lcd_setmode_char(CHRSET_MODE, 4);
        c = (counter & 4 ? CHRSET_MODE : CHR_BLANK);
        break;
#endif

      case mode_slew:
        lcd_custom_char(CHR_CUSTOM_1, CUST_CHR_SLEW, custom_chars);
        c = (blink_phase & 1 ? CHR_BLANK : CHR_CUSTOM_1);
        led_state(RED_LED, (blink_phase & 0x1) == 0);
        break;

      case mode_aux:
        lcd_custom_char(CHR_CUSTOM_1, CUST_CHR_BOMBa, custom_chars);
        int8_t j0 = joystick_get(0);
        int8_t j1 = joystick_get(1);

        if (abs(j0) > abs(j1))
        {
          j1 = 0;
        }
        else
        {
          j0 = 0;
        }

        double angle = 0;

        if ((j0 == 0) && (j1 == 0))
        {
          c = '+';
        }
        else if (j0 > 0)
        {
          c = CHR_RIGHTARROW;
          stepperhw_move(2, angle, time_delta, config.focus.max_speed, config.focus.steps_per_deg); // j0
        }
        else if (j0 < 0)
        {
          c = CHR_LEFTARROW;
          stepperhw_move(2, angle, time_delta, config.focus.max_speed, config.focus.steps_per_deg); // j0
        }
        else if (j1 > 0)
        {
          c = CHR_UPARROW;
          stepperhw_move(3, angle, time_delta, config.aux.max_speed, config.aux.steps_per_deg); // j1
        }
        else if (j1 < 0)
        {
          c = CHR_DNARROW;
          stepperhw_move(3, angle, time_delta, config.aux.max_speed, config.aux.steps_per_deg); // j1
        }
        else
        {
          c = '?';
        }

        led_state(RED_LED, (blink_phase & 0x1) == 0);
        break;

      case mode_goto:
        lcd_custom_char(CHR_CUSTOM_1, CUST_CHR_GOTO, custom_chars);
        c = (blink_phase & 3 ? CHR_BLANK : CHR_CUSTOM_1);
        led_state(RED_LED, 1);
        break;

      default: // display tracking mode if nothing more interesting is going on
        switch (track_mode)
        {
          default:
          case track_idle:
            lcd_custom_char(CHR_CUSTOM_1, CUST_CHR_STOP, custom_chars);
            c = (blink_phase & 0x3 ? CHR_BLANK : CHR_CUSTOM_1);
            led_state(RED_LED, (blink_phase & 0x1) != 0);
            break;

          case track_sidereal:
            lcd_custom_char(CHR_CUSTOM_1, CUST_CHR_CLOCK, custom_chars);
            c = (blink_phase & 0x3 ? CHR_BLANK : CHR_CUSTOM_1);
            led_state(RED_LED, (blink_phase & 0x1f) == 0x10); // blink mid-way
            break;
        }

        break;
    }

    lcd_write_char(15, 0, c);

    // display current angles
    switch (config.mount.type)
    {
      case mount_geq:
#if 0
        tmp = CONV_ANGLE_TO_SEC(modulus_d(
                                  pos_current.ra + CONV_SEC_TO_ANGLE(0.5),
                                  360)); // extra fraction of a second because we are always a bit behind
#endif
        tmp =  CONV_ANGLE_TO_SEC(modulus_d(
                                   pos_current.ra + CONV_SEC_TO_ANGLE(time_delta * 2), 360));
#if 0
        DEBUG_PRINT("tmp ");
        DEBUG_PRINT_FLOAT(tmp);
        DEBUG_PRINT(" pos_current.ra ");
        DEBUG_PRINT_FLOAT(pos_current.ra);
        DEBUG_PRINT(" pos_desired.ra ");
        DEBUG_PRINT_FLOAT(pos_desired.ra);
        DEBUG_PRINT("\n");
#endif
        snprintf(b, sizeof(b), "R/A %3i%c%2i%c%2i%c",
                 (int) CONV_SEC_HR(tmp), 'h',
                 (int) CONV_SEC_MIN(tmp), 'm',
                 (int) CONV_SEC_SEC(tmp), 's');
        b[sizeof(b) - 1] = 0;
        //sprintf(b, "%5.5f", pos_current.dec);
        //sprintf(b, "%i", (int)CONV_ANGLE_TO_ARCSEC(pos_current.dec));
        lcd_write_str(0, 0, b);
        tmp = CONV_ANGLE_TO_ARCSEC(pos_current.dec);

        if (tmp < 0)
        {
          tmp = -tmp;
          snprintf(b2, sizeof(b2), "-%i", (int) CONV_ARCSEC_DEG(tmp));
        }
        else
        {
          snprintf(b2, sizeof(b2), "+%i", (int) CONV_ARCSEC_DEG(tmp));
        }

        snprintf(b, sizeof(b), "DEC%4s%c%2i%c%2i%c",
                 b2, CHR_DEGREE,
                 (int) CONV_ARCSEC_ARCMIN(tmp), CHR_ARCMINUTE,
                 (int) CONV_ARCSEC_ARCSEC(tmp), CHR_ARCSECOND);
        b[sizeof(b) - 1] = 0;
        lcd_write_str(0, 1, b);
        break;

      case mount_altaz:
        alt = CONV_RA_DEC_TO_ALT(pos_current.ra, pos_current.dec);
        az = CONV_RA_DEC_TO_AZ(pos_current.ra, pos_current.dec);
        sprintf(b, "ALT %3i%c%2i%c%2i%c", (int) CONV_ARCSEC_DEG(alt), CHR_DEGREE, (int) CONV_ARCSEC_ARCMIN(alt), CHR_ARCMINUTE, (int) CONV_ARCSEC_ARCSEC(alt), CHR_ARCSECOND);
        lcd_write_str(0, 0, b);
        sprintf(b, "AZI %3i%c%2i%c%2i%c", (int) CONV_ARCSEC_DEG(az), CHR_DEGREE, (int) CONV_ARCSEC_ARCMIN(az), CHR_ARCMINUTE, (int) CONV_ARCSEC_ARCSEC(az), CHR_ARCSECOND);
        lcd_write_str(0, 1, b);
        break;

      default:
        RUNTIME_ERROR("UNHANDLED M-MODE");
        break;
    }

    blink_phase++;
    lcd_release();
    delay_ms(50);
  }

  DEBUG_PRINT("task terminating\n");
  vTaskDelete(NULL);
}

/* wiring

***** CONNECTOR 1 *****

jtag:          pf7, pf5, pf4 (reserved)
spi/stepper:   pb3 (miso), pb2 (mosi), pb1 (sck), pb0 (n/a)
stepper cs:    pb4 (cs)
stepper other: pe2 (reset), pe3 (oc3a - stpclk), pe4 (int 4 - spare), pe5 (int5 - spare), pe6 (int6 - busy/sync), pe7 (int7 - flag)
digital out:   pb5 (oc1a), pb6 (oc1b), pb7 (oc1c - spare)
rs232:         pe0 (rx), pe1 (tx)
joystick 1/2:  pf0 (x), pf1 (y)
joystick 3/4:  pf2 (x) not-connected, pf3 (y) not-connected
battery sense: pf4, pf5 (n-c)

***** CONNECTOR 2 *****

lcd:           pa0-pa7 (data0-data7), pc4-pc7 (rs, rw, en1, en2)
keyboard:      pc0, pc1, pc2, pc3 (button 1-4), pd4, pd5 (n/c)
serial:        pd3 (tx), pd2 (rx), pe1 (tx), pe0 (rx)

other:
extint:        pd0, pd1
counter input: pd6, pd7

***** STEPPER DIN5 *****
1 - B+ (orange)
2 - GND
3 - A- (brown)
4 - B- (yellow)
5 - A+ (black)
SHIELD - GND

 (connector back/plug front)
  +-----+
 /   2   \
|  5   4  |
 \ 3   1 /
  +--v--+

***** AUX/ENC DIN5 *****
1 - In/Out B1 (pd1, pd7, pf5)
//, d?, pc7, pg0, pg1, pe0, pg2, pg3, pg4, pe1, pe4, pc2)
2 - +5V
3 - In/Out A2 (pd2, pf6)
4 - In/Out B2 (pd3, pf7)
5 - In/Out A1 (pd0, pd6, pf4)
SHIELD - GND


*/


static void display_build()
{
  char buffer[LCD_WIDTH + 1];
  sprintf(buffer, "VERSION %i.%i", (int)(CONST_PROJ_VERSION >> 16), (int)(CONST_PROJ_VERSION & 0xffff));
  lcd_write_str(0, 0, buffer);
  PRINT("%s\n", buffer);
  sprintf(buffer, "BUILD %i.%i", (int)(CONST_PROJ_BUILD >> 16), (int)(CONST_PROJ_BUILD & 0xffff));
  PRINT("%s\n", buffer);
  lcd_write_str(0, 1, buffer);
  delay_ms(500);
}

static void service_keypad()
{
  uint8_t red_key_duration = 0;
  uint8_t blue_key_duration = 0;
  uint8_t black_key_duration = 0;

  while (task_active == true)
  {
    if (keypad_iskey(kButtonRED))
    {
      red_key_duration++;

      if (red_key_duration == KEYPAD_LONG_PRESS)
      {
        do_menu(menu_setup, NULL);
        red_key_duration = 0;
      }
    }
    else
    {
      if (red_key_duration != 0)
      {
        if (track_mode == track_sidereal)
        {
          track_mode = track_idle;
        }
        else if (track_mode == track_idle)
        {
          track_mode = track_sidereal;
        }
      }

      red_key_duration = 0;
    }

    if (keypad_iskey(kButtonBLUE))
    {
      blue_key_duration++;

      if (blue_key_duration == KEYPAD_LONG_PRESS)
      {
        do_menu(menu_goto, NULL);
        blue_key_duration = 0;
      }
    }
    else
    {
      if (blue_key_duration != 0)
      {
        neighbors_param_list_valid = false;
        do_menu(menu_neighbour, neighbors_param_list);

        if (neighbors_param_list_valid == true)
        {
          do_menu(menu_goto, neighbors_param_list);
        }
      }

      blue_key_duration = 0;
    }

    if (keypad_iskey(kButtonBLACK))
    {
      //xx
      black_key_duration++;
      op_mode = mode_aux;
    }
    else
    {
      if ((op_mode == mode_aux)
          && (black_key_duration < KEYPAD_LONG_PRESS))
      {
        do_menu(menu_dew_heater, NULL);
      }

      //op_mode = mode_none;
      black_key_duration = 0;
    }

    delay_ms(10);
  }

  DEBUG_PRINT("task terminating\n");
  vTaskDelete(NULL);
}

// set runtime parameters from the current config
static void set_parameters(void)
{
  track_mode = config.mount.init_track_mode;
  stepperhw_set_pwm(0, config.ra.pwm_drive);
  stepperhw_set_pwm(1, config.dec.pwm_drive);
  //stepperhw_set_pwm(2, config.focus.pwm_drive);
  //stepperhw_set_pwm(3, config.aux.pwm_drive);
}

static void guider_main(void *pvParameters)
{
  uint8_t powerup_key = kButtonNoKey;
  // essential service and driver startup
  adc_init();
  joystick_init();
  keypad_init();
  rtc_init();
  led_init();
  lcd_init();
  lcd_acquire();
  display_logo();
  powerup_key = keypad_get();
  display_build();
  lcd_clear();
  lcd_write_str(0, 0, "INITIALIZING");
  lcd_write_str(0, 1, "(WAIT)");
  // hw startup
  eeprom_init(); // need eeprom before stepper
  config_load(&config);
  spi_init();
  stepperhw_init();
  lcd_clear();
  lcd_release();
  // other services startup
  db_init();
  // commit config parameters
  set_parameters();
  // initialize state variables
  heater_power = heater_off;

  if (powerup_key != kButtonNoKey)
  {
    while (1)
    {
      do_menu(menu_photo, NULL);
    }
  }

  // clock set
  rtc_reset();
  rtc_set(config.time.time);
  // reset angles
  pos_current.ra = pos_desired.ra = 0;
  pos_current.dec = pos_desired.dec = 0;
  // set termination flag
  task_active = true;

  // Create the DISPLAY task.
  if (xTaskCreate(update_display, "GDR_UI", GUIDEUI_STACKSIZE, NULL,
                  tskIDLE_PRIORITY + PRIORITY_ASTRO_TASK, NULL) != pdTRUE)
  {
    return ;
  }

#if 1

  // Create the KEYPAD task.
  if (xTaskCreate(service_keypad, "GDR_KEY", GUIDEKEY_STACKSIZE, NULL,
                  tskIDLE_PRIORITY + PRIORITY_ASTRO_TASK, NULL) != pdTRUE)
  {
    return ;
  }

#endif

  // main loop
  while (task_active == true)
  {
    update_coordinates();
    delay_ms(10);
  }

  DEBUG_PRINT("task terminating\n");
  vTaskDelete(NULL);
}

// Initializes the ASTRO service.
unsigned long guider_svc_init(void)
{
  DEBUG_PRINT("Guider service initializing ...\n");

  // Create the ASTRO task.
  if (xTaskCreate(guider_main, "GDR", GUIDER_STACKSIZE, NULL,
                  tskIDLE_PRIORITY + PRIORITY_ASTRO_TASK, NULL) != pdTRUE)
  {
    return (1);
  }

  // Success.
  return (0);
}

// remote control command
void guider_cmd(astro_cmd_e cmd, ...)
{
  va_list ap;
  track_mode_e new_track_mode;
  heater_e new_heater_power;
  va_start(ap, cmd);

  switch (cmd)
  {
    case cmd_nop:
      break;

    case cmd_ra:
      pos_desired.ra += va_arg(ap, double);
      break;

    case cmd_dec:
      pos_desired.dec += va_arg(ap, double);
      break;

    case cmd_goto:
      pos_desired.ra = modulus_d(va_arg(ap, double) + pos_reference.ra, 360);
      pos_desired.dec = modulus_d(va_arg(ap, double) + pos_reference.dec, 360);
      DEBUG_PRINT("GOTO START: to");
      DEBUG_PRINT(" ra ");
      DEBUG_PRINT_FLOAT(pos_desired.ra);
      DEBUG_PRINT(", dec ");
      DEBUG_PRINT_FLOAT(pos_desired.dec);
      DEBUG_PRINT(" from ra ");
      DEBUG_PRINT_FLOAT(pos_current.ra);
      DEBUG_PRINT(", dec ");
      DEBUG_PRINT_FLOAT(pos_current.dec);
      DEBUG_PRINT("\n");
      op_mode = mode_goto;
      break;

    case cmd_align:
      pos_reference.ra = modulus_d(CONV_SEC_TO_ANGLE(va_arg(ap, double)) - pos_current.ra, 360);
      pos_reference.dec = modulus_d(CONV_ARCSEC_TO_ANGLE(va_arg(ap, double)) - pos_current.dec, 360);
      /*
            pos_desired.ra = pos_current.ra = modulus_d(
                va_arg(ap, double) + CONV_SEC_TO_ANGLE(rtc_get()),
                360);
            pos_desired.dec = pos_current.dec = va_arg(ap, double);
      */
      break;

    case cmd_track_mode:
      new_track_mode = va_arg(ap, int);

      if (new_track_mode < track_max)
      {
        track_mode = new_track_mode;
      }

      break;

    case cmd_heater:
      new_heater_power = va_arg(ap, int);

      if (new_heater_power < heater_max)
      {
        heater_power = new_heater_power;
        rtc_pwm_enable(0, CONST_TICKS_PER_SECOND * 4, heater_power * 25);
      }

      break;

    case cmd_terminate:
      task_active = false;
      break;

    default:
      DEBUG_PRINT("ERROR: Unknown command %i\n", cmd);
  }

  va_end(ap);
}
