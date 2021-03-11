/*
* IncFile1.h
*
* Created: 30/10/2012 1:46:36 PM
*  Author: michal
*/


#ifndef STEPPER_H_
#define STEPPER_H_

#define DSPIN_BUGFIX            // KICK the dspin periodically if at low constant speeds to prevent overcurrent
#define DSPIN_BUGFIX_TIMEOUT    600 // seconds

//#define MOTOR_SIMULATION        // pretend motors

#define CONST_NUM_MOTORS        2 // maximum number of motors
#define CONST_SLEW_SPEED        CONST_MAX_STEPS_PER_SEC

#define CONST_STEPPER_TEETH           15
#define CONST_SPROCKET_TEETH          122
#define CONST_MOTOR_DEG_PER_STEP      1.8
#define CONST_STEPS_PER_ROTATION      (360 / CONST_MOTOR_DEG_PER_STEP) // = 200
// from http://www.cloudynights.com/ubbarchive/showflat.php/Cat/0/Number/1679991
#define CONST_MOUNT_RA_TEETH        144
// note: gp mount needs 1/10 rpm = 1/600 rps, steps per sec =
#define CONST_STEPS_PER_MOUNT_DEG                   /* # steps per degree of mount movement */ \
     (CONST_STEPS_PER_ROTATION *                    /* motor steps per full turn */ \
     (CONST_SPROCKET_TEETH / CONST_STEPPER_TEETH) * /* transmission */ \
     (CONST_MOUNT_RA_TEETH / 360.0))                /* turns per degree of movement */

#define CONST_MICROSTEPS_PER_STEP     128
#define CONST_MAX_STEPS_PER_SEC       3600 // 1800
#define CONST_DEFAULT_STEPS_PER_SEC   900
#define CONST_MIN_STEPS_PER_SEC       10  // sidereal is about 2.58
#define CONST_STALL_CURRENT           1000
#define CONST_OVER_CURRENT            1000
#define CONST_FULL_STEPPING_THRESHOLD 1000 // enable all the time: lower current consumption & less noise
#define CONST_ACCELERATION            0xfff // max
#define CONST_DECCELERATION           0xfff // max
#define CONST_PWM_DRIVE_RUN           28
#define CONST_PWM_DRIVE_ACC           28
#define CONST_PWM_DRIVE_DEC           28
#define CONST_PWM_DRIVE_HOLD          28

enum
{
  MOTOR_STATE_STOPPED = 0,
  MOTOR_STATE_FWD,           // 1
  MOTOR_STATE_FWD_STOPPING,  // 2
  MOTOR_STATE_REV,           // 3
  MOTOR_STATE_REV_STOPPING   // 4
} motor_state_e;

void stepperhw_init_dev(uint8_t dev);
void stepperhw_init();
double stepperhw_move(uint8_t dev, double angle, double period, uint16_t max_speed, uint32_t steps_per_deg);
void stepperhw_test(uint32_t argument);
void stepperhw_print_status(uint8_t dev);
void stepperhw_check_status(void);
uint8_t stepperhw_calib(uint8_t dev);
void stepperhw_set_pwm(uint8_t dev, uint8_t val);
void stepperhw_safe_mode(void);

#endif /* STEPPER_H_ */
