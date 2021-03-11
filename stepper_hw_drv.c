/*
* CFile1.c
*
* Created: 25/09/2012 10:00:40 PM
*  Author: michal
*/

#include "project.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static uint8_t motor_state[CONST_NUM_MOTORS];
static uint32_t motor_pos[CONST_NUM_MOTORS];
static uint8_t motor_kval_0rpm[CONST_NUM_MOTORS];

void stepperhw_init()
{
  DEBUG_PRINT("HW stepper driver init\n");
  uint8_t i;

  //spi_init();

  for (i = 0; i < CONST_NUM_MOTORS; i++)
  {
    stepperhw_init_dev(i);
  }
}

void stepperhw_print_status(uint8_t dev)
{
  uint32_t rc = 0;

  if (dev >= CONST_NUM_MOTORS)
  {
    return;
  }

  dspin_select(dev);
  rc = dspin_getStatus();
  PRINT("Stepper %i status 0x%x\n", dev, rc);
}

void stepperhw_safe_mode(void)
{
  for (uint8_t i = 0; i < CONST_NUM_MOTORS; i++)
  {
    dspin_select(i);
    dspin_free();
  }
}

void stepperhw_check_status(void)
{
  const uint32_t error_mask = (STATUS_UVLO | STATUS_TH_WRN | STATUS_TH_SD | STATUS_OCD | STATUS_STEP_LOSS_A | STATUS_STEP_LOSS_B);

  for (uint8_t i = 0; i < CONST_NUM_MOTORS; i++)
  {
    uint32_t status;
    dspin_select(i);
    status = dspin_getStatus();

    // detect a problem
    if ((status != 0) && (status & error_mask) != error_mask)
    {
      char msg1[20 + 1];
      char *msg2 = NULL;
      stepperhw_safe_mode();
      sprintf(msg1, "STEP CTL%u 0x%04x", (unsigned int)i, (unsigned int)status);

      if ((status & STATUS_UVLO) == 0)
      {
        msg2 = "UNDERVOLTAGE";
      }
      else if ((status & STATUS_TH_WRN) == 0)
      {
        msg2 = "THERMAL WARN";
      }
      else if ((status & STATUS_TH_SD) == 0)
      {
        msg2 = "THERMAL SHUTDN";
      }
      else if ((status & STATUS_OCD) == 0)
      {
        msg2 = "OVERCURRENT";
      }
      else if ((status & STATUS_STEP_LOSS_A) == 0)
      {
        msg2 = "A-BRIDGE STALL";
      }
      else if ((status & STATUS_STEP_LOSS_B) == 0)
      {
        msg2 = "B-BRIDGE STALL";
      }
      else
      {
        msg2 = "FAULT STATUS";
      }

      PRINT("stepperhw_check_status: %s %s\n", msg1, msg2);
      runtime_error(msg1, msg2);
    }
  }
}

void stepperhw_init_dev(uint8_t dev)
{
  if (dev >= CONST_NUM_MOTORS)
  {
    return;
  }

  motor_state[dev] = MOTOR_STATE_STOPPED;
  dspin_select(dev);
  dspin_resetDev();

  if (dspin_init() == 0)
  {
    return; // error
  }

  stepperhw_calib(dev);
  //dspin_init();
  dspin_free(); //disengage motors
  dspin_setMicroSteps(CONST_MICROSTEPS_PER_STEP);  //1,2,4,8,16,32,64 or 128
  dspin_setAcc(CONST_ACCELERATION);  //set acceleration
  dspin_setDec(CONST_DECCELERATION);
  dspin_setMaxSpeed(CONST_MAX_STEPS_PER_SEC);
  dspin_setMinSpeed(CONST_MIN_STEPS_PER_SEC);
  dspin_setThresholdSpeed(CONST_FULL_STEPPING_THRESHOLD);
  dspin_setOverCurrent(CONST_OVER_CURRENT);  //set overcurrent protection
  dspin_setStallCurrent(CONST_STALL_CURRENT);
  dspin_SetLowSpeedOpt(1, CONST_MIN_STEPS_PER_SEC);
  // PWM settings
  dspin_SetParam(KVAL_RUN, CONST_PWM_DRIVE_RUN);
  dspin_SetParam(KVAL_ACC, CONST_PWM_DRIVE_ACC);
  dspin_SetParam(KVAL_DEC, CONST_PWM_DRIVE_DEC);
  dspin_SetParam(KVAL_HOLD, CONST_PWM_DRIVE_HOLD);
  // ALARM settings
  dspin_SetParam(ALARM_EN, ALARM_EN_OVERCURRENT | ALARM_EN_THERMAL_SHUTDOWN | ALARM_EN_THERMAL_WARNING | ALARM_EN_UNDER_VOLTAGE);
  // ALARM_EN_STALL_DET_A | ALARM_EN_STALL_DET_B | ALARM_EN_SW_TURN_ON  | ALARM_EN_WRONG_NPERF_CMD
  // ST_SLP
  // K_THERM
  dspin_getStatus();
  //dspin_hardStop(); //engage motors
  dspin_free(); //disengage motors
}

// move stepper by angle (degrees) within a given time period (sec)
// unless overriden by a motion vector
// return the actual angle moved in the angle variable
// angle is per-second
double stepperhw_move(uint8_t dev, double angle, /*double limit, int8_t vector,*/ double period, uint16_t max_speed, uint32_t steps_per_deg)
{
  if (dev >= CONST_NUM_MOTORS)
  {
    return (0);
  }

#ifndef MOTOR_SIMULATION
  uint8_t dir = -1;
  double speed = 0;
  int32_t pos = 0, pos_delta;
  // calculate motor parameters
  //speed = CONST_STEPS_PER_MOUNT_DEG * angle / period;
  speed = ((double)steps_per_deg * angle) / period;

/*
  if (vector != 0)
  {
    speed += + (max_speed * (float) vector) / 100.0;
  }
*/

  dir = (speed > 0) ? FWD : REV;
  speed = ABS(speed);
  //speed = MIN(speed, CONST_MAX_STEPS_PER_SEC);
  speed = MIN(speed, max_speed);
  // send stepper command
  dspin_select(dev);
#ifdef DSPIN_BUGFIX

  // overcurrent bug fix
  if (speed != 0)
  {
    static double t[CONST_NUM_MOTORS] = {0, 0};
//    static double min_speed[CONST_NUM_MOTORS] = {+INFINITY, +INFINITY};
#if 0

    // calculate lowest constant speed
    if (ABS(speed) <= min_speed[dev])
    {
      min_speed[dev] = MIN(min_speed[dev], ABS(speed));
      DEBUG_PRINT("BUGFIX: DSPIN %i min_speed ", dev);
      DEBUG_PRINT_FLOAT(min_speed[dev]);
      DEBUG_PRINT("\n");
    }

#endif

    // detect when at lowest speed
    if (ABS(speed) < CONST_MIN_STEPS_PER_SEC) // (2 * min_speed[dev]))
    {
      if (t[dev] >= DSPIN_BUGFIX_TIMEOUT) // seconds timeout
      {
        dspin_hardStop();
        DEBUG_PRINT("BUGFIX: DSPIN %i KICK\n", dev);
        t[dev] = 0;
      }
      else
      {
        t[dev] += period; // advance timeout
      }
    }
    else
    {
      t[dev] = 0; // reset timeout if not at lowest speed
    }
  }

#endif //DSPIN_BUGFIX

  //{ float f = CONST_MICROSTEPS_PER_DEG; DEBUG_PRINT("MWD: %i\n", (uint32_t)f); }

  switch (motor_state[dev])
  {
    case MOTOR_STATE_STOPPED:
      switch (dir)
      {
        case FWD:
          dspin_run(dir, speed);
#if 0
          DEBUG_PRINT("stepperhw_move run dev %i, dir %i, speed ", dev, dir);
          DEBUG_PRINT_FLOAT(*speed);
          DEBUG_PRINT("\n");
#endif
          motor_state[dev] = MOTOR_STATE_FWD;
          break;

        case REV:
          dspin_run(dir, speed);
#if 0
          DEBUG_PRINT("stepperhw_move run dev %i, dir %i, speed ", dev, dir);
          DEBUG_PRINT_FLOAT(*speed);
          DEBUG_PRINT("\n");
#endif
          motor_state[dev] = MOTOR_STATE_REV;
          break;

        default:
          // nop
          break;
      }

      break;

    case MOTOR_STATE_FWD:
      switch (dir)
      {
        case FWD:
          dspin_run(dir, speed);
#if 0
          DEBUG_PRINT("stepperhw_move run dev %i, dir %i, speed ", dev, dir);
          DEBUG_PRINT_FLOAT(speed);
          DEBUG_PRINT("\n");
#endif
//      DEBUG_PRINT("stepperhw_move FWD getpos dev %i, pos %i\n", dev, pos);
          break;

        case REV:
          dspin_softStop();
          motor_state[dev] = MOTOR_STATE_FWD_STOPPING;
          //DEBUG_PRINT("*** FWD->REV\n");
          break;

        default:
          // nop
          break;
      }

      break;

    case MOTOR_STATE_REV:
      switch (dir)
      {
        case FWD:
          dspin_softStop();
          motor_state[dev] = MOTOR_STATE_REV_STOPPING;
          //DEBUG_PRINT("*** REV->FWD\n");
          break;

        case REV:
          dspin_run(dir, speed);
#if 0
          DEBUG_PRINT("stepperhw_move run dev %i, dir %i, speed ", dev, dir);
          DEBUG_PRINT_FLOAT(speed);
          DEBUG_PRINT("\n");
#endif
//      DEBUG_PRINT("stepperhw_move REV getpos dev %i, pos %i\n", dev, pos);
          break;

        default:
          // nop
          break;
      }

      break;

      //todo: add running in the same direction
    case MOTOR_STATE_FWD_STOPPING:
    case MOTOR_STATE_REV_STOPPING:
      if (dspin_isBusy() == 1)
      {
        break;
      }

      motor_state[dev] = MOTOR_STATE_STOPPED;
      //DEBUG_PRINT("*** ->STOP\n");
      break;
  }

  // recalculate the angle, account for wrap-around
  const uint32_t num_bits = 22;
  pos = dspin_getPos(); // 22 bit result
  pos_delta = (pos - motor_pos[dev]) & ((1 << num_bits) - 1);
  pos_delta -= ((pos_delta >> (num_bits - 1)) == 0) ? 0 : 1 << num_bits; // adjust if negative
#if 0

  if (dev == 0)
  {
    //DEBUG_PRINT("%i - %i = %i\n", pos, motor_pos[dev], pos_delta);
    {
      static uint32_t i = 0;

      if (i == 10)
      {
        DEBUG_PRINT("%s vector %i, state %i, pos %i, pos_delta %i", (dev == 0) ? "RA" : "DEC",
                    vector, motor_state[dev], pos, pos_delta);
        DEBUG_PRINT(", period ");
        DEBUG_PRINT_FLOAT(period);
        DEBUG_PRINT(", speed %s", dir == FWD ? "(F) " : "(R) ");
        DEBUG_PRINT_FLOAT(speed);
        DEBUG_PRINT(", angle_asked ");
        DEBUG_PRINT_FLOAT(angle);
        angle = ((double) pos_delta) / (float)(CONST_MICROSTEPS_PER_STEP * CONST_STEPS_PER_MOUNT_DEG);
        DEBUG_PRINT(", angle_moved ");
        DEBUG_PRINT_FLOAT(angle);
        DEBUG_PRINT("\n");
        i = 0;
      }
      else
      {
        i++;
      }
    }
  }

#endif
  motor_pos[dev] = pos;
  //return ((double) pos_delta) / (double)(CONST_MICROSTEPS_PER_STEP * CONST_STEPS_PER_MOUNT_DEG);
  return ((double) pos_delta) / (double)(CONST_MICROSTEPS_PER_STEP * steps_per_deg);
#else /* MOTOR_SIMULATION */
  double delta;

  if (*angle > 0.1010101)
  {
    *angle = 0.1010101;
  }
  else if (*angle < -0.1010101)
  {
    *angle = -0.1010101;
  }

  delta = ((double) sign_d(vector)) * ((double) vector) * ((double) vector) / 1000;

  if (delta > 0.1010101)
  {
    delta = 0.1010101;
  }
  else if (delta < -0.1010101)
  {
    delta = -0.1010101;
  }

  *angle += delta;
  return;
#endif /* !MOTOR_SIMULATION */
}

inline uint8_t _calib(double speed)
{
  uint8_t i;
  const uint32_t error_mask = (STATUS_OCD /*| STATUS_STEP_LOSS_A | STATUS_STEP_LOSS_B */);
  uint32_t status = error_mask;

  for (i = 0; (i < 255) && ((status & error_mask) == error_mask); i++)
  {
    //uint32_t rc;
    //DEBUG_PRINT("(%i)", i);
    dspin_SetParam(KVAL_HOLD, i);
    dspin_SetParam(KVAL_RUN, i);
    dspin_SetParam(KVAL_ACC, i);
    dspin_SetParam(KVAL_DEC, i);

    if (speed > 0)
    {
      dspin_run(FWD, speed);
    }
    else
    {
      dspin_hardStop(); //engage motors
    }

    // wait for motor
    //while (((rc = dspin_GetParam(STATUS_MOT_STATUS)) != STATUS_MOT_STATUS_CONST_SPD) && (rc != STATUS_MOT_STATUS_STOPPED))

    while (dspin_isBusy())
    {
      delay_ms(1);
    }

    delay_ms(10);
    status = dspin_getStatus();
  }

  dspin_SetParam(KVAL_HOLD, 0);
  return (i);
}

uint8_t stepperhw_calib(uint8_t dev)
{
  const uint32_t current = 1000; // mA
  //uint32_t steps_per_sec = 0;
  uint32_t calib_val;
  //spi_init();
  //stepperhw_init();
  //dspin_setAcc(100);  //set acceleration
  //dspin_setDec(100);
  DEBUG_PRINT("Stepper_hw device %i calibration\n", dev);
  dspin_select(dev);
  //for (current = 100; current <= 1000; current += 100)
  //{
  dspin_setOverCurrent(current);
  dspin_setStallCurrent(current);
  calib_val = _calib(0);
  DEBUG_PRINT("MAX KVAL %i @ %imA\n", calib_val, current);
  //}
#if 0

  do
  {
    calib_val = _calib(steps_per_sec);
    DEBUG_PRINT("KVAL %i @ %imA, %i steps/sec\n", calib_val, current, steps_per_sec);
    steps_per_sec = ((steps_per_sec * 10) / 9) + 1;
  }
  while ((calib_val > 0) && (steps_per_sec < 500));

#endif
  dspin_free(); //disengage motors
  motor_kval_0rpm[dev] = calib_val;
  return (calib_val);
}

void stepperhw_set_pwm(uint8_t dev, uint8_t val)
{
  uint16_t i;

  if (dev >= CONST_NUM_MOTORS)
  {
    return;
  }

  i = motor_kval_0rpm[dev];
  i = (i * val) / 200; // 50% * val * calibration value
  i = MIN(motor_kval_0rpm[dev], i);
  DEBUG_PRINT("stepperhw_set_pwm: dev %i, val %i, kval %i\n", dev, val, i);
  dspin_select(dev);
  dspin_free(); //disengage motors
  dspin_SetParam(KVAL_DEC, i);
  dspin_SetParam(KVAL_HOLD, i);
  dspin_SetParam(KVAL_RUN, i);
  dspin_SetParam(KVAL_ACC, i);
  dspin_softStop(); //engage motors
}

void stepperhw_test(uint32_t argument)
{
  spi_init();
  stepperhw_init();
  DEBUG_PRINT("Stepper_hw test ");
  dspin_select(0);
#if 0

  while (dspin_isBusy())
  {
    delay_ms(1);
  }

#endif
  DEBUG_PRINT("(move+)");
  dspin_move(1000 * CONST_MICROSTEPS_PER_STEP);

  while (dspin_isBusy())
  {
    delay_ms(1);
  }

  DEBUG_PRINT("(move-)");
  dspin_move(-1000 * CONST_MICROSTEPS_PER_STEP);

  while (dspin_isBusy())
  {
    delay_ms(1);
  }

  DEBUG_PRINT("(rev)");
  dspin_run(REV, 100);

  while (dspin_isBusy())
  {
    delay_ms(1);
  }

  delay_ms(5000);
  DEBUG_PRINT("(fwd)");
  dspin_run(FWD, 100);

  while (dspin_isBusy())
  {
    delay_ms(1);
  }

  delay_ms(5000);
  dspin_free();
}
