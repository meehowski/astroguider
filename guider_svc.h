/*
 * IncFile1.h
 *
 * Created: 16/12/2012 6:43:29 PM
 *  Author: michal
 */


#ifndef ASTROGUIDER_H_
#define ASTROGUIDER_H_

// lonng keypress duration
#define KEYPAD_LONG_PRESS 100

// custom character definitions
#define CUST_CHR_LEVELa   0
#define CUST_CHR_LEVELz   7
#define CUST_CHR_BOMBa    8
#define CUST_CHR_BOMBz    15
#define CUST_CHR_STOP   16
#define CUST_CHR_MOON   17
#define CUST_CHR_STAR   18
#define CUST_CHR_PLANET   19
#define CUST_CHR_SUN    20
#define CUST_CHR_SLEW   21
#define CUST_CHR_CLOCK    22
#define CUST_CHR_GOTO   23
#define CUST_CHR_UPDN   24
#define CUST_CHR_BATa   25
#define CUST_CHR_BATb   26
#define CUST_CHR_BATc   27
#define CUST_CHR_BATd   28
#define CUST_CHR_BATe   29
#define CUST_CHR_BATf   30
#define CUST_CHR_BATg   31
#define CUST_CHR_HEATER   32

typedef enum
{
  mount_geq = 0,
  mount_altaz,
  mount_undefined
} mount_type_e ;

typedef enum
{
  dir_forward = 0,
  dir_backward = 1
} motor_dir_e ;

// tracking modes
typedef enum
{
  track_idle      = 0,
  track_sidereal,
  track_max
} track_mode_e;

// operating modes
typedef enum
{
  mode_none = 0,
  mode_menu,
  mode_slew,
  mode_aux,
  mode_goto
} op_mode_e;

typedef enum
{
  bat_8 = 0,
  bat_7,
  bat_6,
  bat_5,
  bat_4,
  bat_3,
  bat_2,
  bat_1,
  bat_undefined
} bat_state_e;

enum
{
  menu_setup = 0,
  menu_goto,
  menu_neighbour,
  menu_dew_heater,
  menu_photo
};

enum
{
  panorama_orientation_vertical = 0,
  panorama_orientation_horizontal = 1
};

// stepper configuration
typedef struct
{
  int32_t /*motor_dir_e*/ dir; // motor orientation/direction
  int32_t steps_per_deg;       // motor steps per degree of movement
  int32_t max_speed;           // 1-65535 rpm
  int32_t pwm_drive;           // 1-100% * calib value
  pidcfg_t  pid;
} stepper_config_t;

typedef enum
{
  ra_unlimited = 0,
  ra_limit_360deg
} limit_ra_e;

typedef enum
{
  dec_neg90__to_pos90 = 0,
  dec_0_to_pos90
} limit_dec_e;

// mount configuration
typedef struct
{
  int32_t /* limit_ra_e */   limit_ra;        // 0: unlimited, 1: ra movement limit at 360deg TODO
  int32_t /* limit_dec_e */  limit_dec;       // 0: -90...+90, 1: dec axis flip enabled for -ive angles TODO
  int32_t /* mount_type_e */ type;            // type of mount used
  int32_t /* track_mode_e */ init_track_mode; // initial track mode
} mount_config_t;

// photo panorama settings
// mount configuration
typedef struct
{
  int32_t profile;
  int32_t orientation;
  int32_t cropfactor;
  int32_t degpershot;
  int32_t numdegrees;
  int32_t waitpershot;
  int32_t motor_stepsdeg;
} photo_config_t;

typedef struct
{
  int32_t latitude;
  int32_t longitude;
  int32_t whereami_db_num;
} location_config_t;

// time settings
typedef struct
{
  int32_t time;
  int32_t date;
} time_config_t;

typedef struct
{
  int32_t db_id;
  int32_t obj_id;
} db_info_t ;

typedef struct
{
  double ra;
  double dec;
} coord_info_t ;

typedef enum
{
  cmd_nop = 0,
  cmd_ra,
  cmd_dec,
  cmd_goto,
  cmd_align,
  cmd_track_mode,
  cmd_heater,
  cmd_terminate
} astro_cmd_e;

typedef enum
{
  heater_off = 0,
  heater_25p,
  heater_50p,
  heater_75p,
  heater_on,
  heater_max
} heater_e;

// configuration structure
typedef struct
{
  location_config_t location;
  time_config_t     time;

  stepper_config_t  ra;
  stepper_config_t  dec;
  stepper_config_t  focus;
  stepper_config_t  aux;

  mount_config_t    mount;

  photo_config_t    photo;

//  heater_e          heater1;
//  heater_e          heater2;
  uint32_t          magick; // keep this at the end
} config_t;

extern config_t config;

unsigned long guider_svc_init(void);
void set_pid(float p, float i, float d, float w, uint32_t index);
void guider_cmd(astro_cmd_e cmd, ...);

#endif /* ASTROGUIDER_H_ */
