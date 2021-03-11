////////////////////////////////////////////////////////////
//ORIGINAL CODE 12/12/2011- Mike Hord, SparkFun Electronics
//LIBRARY Created by Adam Meyer of bildr Aug 18th 2012
//Released as MIT license
////////////////////////////////////////////////////////////

#include "project.h"

uint32_t g_device_index;

void dspin_select(uint8_t device_index)
{
  g_device_index = device_index;
}

// This is the generic initialization function to set up the dspin controller
uint8_t dspin_init()
{
  unsigned int i = -1;

  // First things first: let's check communications. The CONFIG register should
  //  power up to 0x2E88, so we can use that to check the communications.
  if ((i = dspin_GetParam(CONFIG)) == 0x2E88)
  {
    DEBUG_PRINT("dspin %i init OK\n", g_device_index);
  }
  else
  {
    DEBUG_PRINT("dspin %i comm error, cfg = 0x%x\n", g_device_index, i);
    return (0);
  }

  // First, let's set the step mode register:
  //   - SYNC_EN controls whether the BUSY/SYNC pin reflects the step
  //     frequency or the BUSY status of the chip. We want it to be the BUSY
  //     status.
  //   - STEP_SEL_x is the microstepping rate- we'll go full step.
  //   - SYNC_SEL_x is the ratio of (micro)steps to toggles on the
  //     BUSY/SYNC pin (when that pin is used for SYNC). Make it 1:1, despite
  //     not using that pin.
  dspin_SetParam(STEP_MODE, (!SYNC_EN) | STEP_SEL_1 | SYNC_SEL_1);
  // Set up the CONFIG register as follows:
  //  PWM base freq = 31.25kHz
  //  PWM frequency divisor = 1
  //  PWM frequency multiplier = 2 (62.5kHz PWM frequency)
  //  Slew rate is 320V/us
  //  Do shut down bridges on overcurrent
  //  Enable motor voltage compensation
  //  Hard stop on switch low
  //  16MHz internal oscillator, nothing on output
  dspin_SetParam(CONFIG, CONFIG_PWM_DIV_4 | CONFIG_PWM_MUL_2 | CONFIG_SR_320V_us | CONFIG_OC_SD_ENABLE | CONFIG_VS_COMP_ENABLE | CONFIG_SW_HARD_STOP | CONFIG_INT_16MHZ);
  // Configure the RUN KVAL. This defines the duty cycle of the PWM of the bridges
  //  during running. 0xFF means that they are essentially NOT PWMed during run; this
  //  MAY result in more power being dissipated than you actually need for the task.
  //  Setting this value too low may result in failure to turn.
  //  There are ACC, DEC, and HOLD KVAL registers as well; you may need to play with
  //  those values to get acceptable performance for a given application.
  dspin_SetParam(KVAL_RUN, 0xFF);
  dspin_SetParam(KVAL_ACC, 0xFF);
  dspin_SetParam(KVAL_DEC, 0xFF);
  dspin_SetParam(KVAL_HOLD, 0xFF);
  // Calling dspin_GetStatus() clears the UVLO bit in the status register, which is set by
  //  default on power-up. The driver may not run without that bit cleared by this
  //  read operation.
  dspin_getStatus();
  //dspin_hardStop(); //engage motors
  dspin_free(); //do not engage motors
  return (1);
}

uint8_t dspin_isBusy()
{
  uint32_t status = dspin_getStatus();
  return !((status >> 1) & 0x1);
}

void dspin_setMicroSteps(uint32_t microSteps)
{
  uint8_t stepVal;

  for (stepVal = 0; stepVal < 8; stepVal++)
  {
    if (microSteps == 1)
    {
      break;
    }

    microSteps = microSteps >> 1;
  }

  dspin_SetParam(STEP_MODE, (!SYNC_EN) | stepVal | SYNC_SEL_1);
}

void dspin_setThresholdSpeed(float thresholdSpeed)
{
  // Configure the FS_SPD register- this is the speed at which the driver ceases
  //  microstepping and goes to full stepping. FSCalc() converts a value in steps/s
  //  to a value suitable for this register; to disable full-step switching, you
  //  can pass 0x3FF to this register.
  if (thresholdSpeed == 0.0)
  {
    dspin_SetParam(FS_SPD, 0x3FF);
  }
  else
  {
    dspin_SetParam(FS_SPD, dspin_FSCalc(thresholdSpeed));
  }
}


void dspin_setCurrent(uint32_t current)
{
}


void dspin_setMaxSpeed(uint32_t speed)
{
  // Configure the MAX_SPEED register- this is the maximum number of (micro)steps per
  //  second allowed. You'll want to mess around with your desired application to see
  //  how far you can push it before the motor starts to slip. The ACTUAL parameter
  //  passed to this function is in steps/tick; dspin_MaxSpdCalc() will convert a number of
  //  steps/s into an appropriate value for this function. Note that for any move or
  //  goto type function where no speed is specified, this value will be used.
  dspin_SetParam(MAX_SPEED, dspin_MaxSpdCalc(speed));
}


void dspin_setMinSpeed(uint32_t speed)
{
  // Configure the MAX_SPEED register- this is the maximum number of (micro)steps per
  //  second allowed. You'll want to mess around with your desired application to see
  //  how far you can push it before the motor starts to slip. The ACTUAL dspin_Parameter
  //  passed to this function is in steps/tick; dspin_MaxSpdCalc() will convert a number of
  //  steps/s into an appropriate value for this function. Note that for any move or
  //  goto type function where no speed is specified, this value will be used.
  //dspin_SetParam(MIN_SPEED, dspin_MinSpdCalc(speed) | (SetLowSpeedOpt !=0 ? 0x1000 : 0));
  dspin_SetParam(MIN_SPEED, dspin_MinSpdCalc(speed));
}




void dspin_setAcc(float acceleration)
{
  // Configure the acceleration rate, in steps/tick/tick. There is also a DEC register;
  //  both of them have a function (AccCalc() and DecCalc() respectively) that convert
  //  from steps/s/s into the appropriate value for the register. Writing ACC to 0xfff
  //  sets the acceleration and deceleration to 'infinite' (or as near as the driver can
  //  manage). If ACC is set to 0xfff, DEC is ignored. To get infinite deceleration
  //  without infinite acceleration, only hard stop will work.
  uint32_t accelerationBYTES = dspin_AccCalc(acceleration);
  dspin_SetParam(ACC, accelerationBYTES);
}


void dspin_setDec(float deceleration)
{
  uint32_t decelerationBYTES = dspin_DecCalc(deceleration);
  dspin_SetParam(DEC, decelerationBYTES);
}


uint32_t dspin_getPos()
{
  uint32_t position = dspin_GetParam(ABS_POS);
  return dspin_convert(position);
}

float dspin_getSpeed()
{
  /*
   SPEED
  The SPEED register contains the current motor speed, expressed in step/tick (format unsigned fixed pouint32_t 0.28).
  In order to convert the SPEED value in step/s the following formula can be used:
  Equation 4
  where SPEED is the integer number stored into the register and tick is 250 ns.
  The available range is from 0 to 15625 step/s with a resolution of 0.015 step/s.
  Note: The range effectively available to the user is limited by the MAX_SPEED dspin_Parameter.
  */
  return (float) dspin_GetParam(SPEED);
  //return (float) speed * pow(8, -22);
  //return FSCalc(speed); NEEDS FIX
}


void dspin_setOverCurrent(uint32_t ma_current)
{
  // Configure the overcurrent detection threshold.
  uint8_t OCValue = (uint8_t) floor(ma_current / 375);

  if (OCValue > 0x0F)
  {
    OCValue = 0x0F;
  }

  dspin_SetParam(OCD_TH, OCValue);
}

void dspin_setStallCurrent(float ma_current)
{
  uint8_t STHValue = (uint8_t) floor(ma_current / 31.25);

  if (STHValue > 0x80)
  {
    STHValue = 0x80;
  }

  if (STHValue < 0)
  {
    STHValue = 0;
  }

  dspin_SetParam(STALL_TH, STHValue);
}

void dspin_SetLowSpeedOpt(uint8_t enable, uint32_t threshold_speed)
{
  // Enable or disable the low-speed optimization option. If enabling,
  //  the other 12 bits of the register will be automatically zero.
  //  When disabling, the value will have to be explicitly written by
  //  the user with a SetParam() call. See the datasheet for further
  //  information about low-speed optimization.
  dspin_Xfer(SET_PARAM | MIN_SPEED);

  if (enable)
  {
    dspin_Param(0x1000 | dspin_MinSpdCalc(threshold_speed), 13);
  }
  else
  {
    dspin_Param(dspin_MinSpdCalc(threshold_speed), 13);
  }
}


void dspin_run(uint8_t dir, float spd)
{
  // RUN sets the motor spinning in a direction (defined by the constants
  //  FWD and REV). Maximum speed and minimum speed are defined
  //  by the MAX_SPEED and MIN_SPEED registers; exceeding the FS_SPD value
  //  will switch the device into full-step mode.
  // The SpdCalc() function is provided to convert steps/s values into
  //  appropriate integer values for this function.
  uint32_t speedVal = dspin_SpdCalc(spd);
  dspin_Xfer(RUN | dir);

  if (speedVal > 0xFFFFF)
  {
    speedVal = 0xFFFFF;
  }

  dspin_Xfer((uint8_t)(speedVal >> 16));
  dspin_Xfer((uint8_t)(speedVal >> 8));
  dspin_Xfer((uint8_t)(speedVal));
}


void dspin_Step_Clock(uint8_t dir)
{
  // STEP_CLOCK puts the device in external step clocking mode. When active,
  //  pin 25, STCK, becomes the step clock for the device, and steps it in
  //  the direction (set by the FWD and REV constants) imposed by the call
  //  of this function. Motion commands (RUN, MOVE, etc) will cause the device
  //  to exit step clocking mode.
  dspin_Xfer(STEP_CLOCK | dir);
}

void dspin_move(int32_t n_step)
{
  // MOVE will send the motor n_step steps (size based on step mode) in the
  //  direction imposed by dir (FWD or REV constants may be used). The motor
  //  will accelerate according the acceleration and deceleration curves, and
  //  will run at MAX_SPEED. Stepping mode will adhere to FS_SPD value, as well.
  uint8_t dir;

  if (n_step >= 0)
  {
    dir =  FWD;
  }
  else
  {
    dir =  REV;
  }

  uint32_t n_stepABS = abs(n_step);
  dspin_Xfer(MOVE | dir);  //set direction

  if (n_stepABS > 0x3FFFFF)
  {
    n_step = 0x3FFFFF;
  }

  dspin_Xfer((uint8_t)(n_stepABS >> 16));
  dspin_Xfer((uint8_t)(n_stepABS >> 8));
  dspin_Xfer((uint8_t)(n_stepABS));
}

void dspin_goTo(uint32_t pos)
{
  // GOTO operates much like MOVE, except it produces absolute motion instead
  //  of relative motion. The motor will be moved to the indicated position
  //  in the shortest possible fashion.
  dspin_Xfer(GOTO);

  if (pos > 0x3FFFFF)
  {
    pos = 0x3FFFFF;
  }

  dspin_Xfer((uint8_t)(pos >> 16));
  dspin_Xfer((uint8_t)(pos >> 8));
  dspin_Xfer((uint8_t)(pos));
}


void dspin_goTo_DIR(uint8_t dir, uint32_t pos)
{
  // Same as GOTO, but with user constrained rotational direction.
  dspin_Xfer(GOTO_DIR);

  if (pos > 0x3FFFFF)
  {
    pos = 0x3FFFFF;
  }

  dspin_Xfer((uint8_t)(pos >> 16));
  dspin_Xfer((uint8_t)(pos >> 8));
  dspin_Xfer((uint8_t)(pos));
}

void dspin_goUntil(uint8_t act, uint8_t dir, uint32_t spd)
{
  // GoUntil will set the motor running with direction dir (REV or
  //  FWD) until a falling edge is detected on the SW pin. Depending
  //  on bit SW_MODE in CONFIG, either a hard stop or a soft stop is
  //  performed at the falling edge, and depending on the value of
  //  act (either RESET or COPY) the value in the ABS_POS register is
  //  either RESET to 0 or COPY-ed into the MARK register.
  dspin_Xfer(GO_UNTIL | act | dir);

  if (spd > 0x3FFFFF)
  {
    spd = 0x3FFFFF;
  }

  dspin_Xfer((uint8_t)(spd >> 16));
  dspin_Xfer((uint8_t)(spd >> 8));
  dspin_Xfer((uint8_t)(spd));
}

void dspin_releaseSW(uint8_t act, uint8_t dir)
{
  // Similar in nature to GoUntil, ReleaseSW produces motion at the
  //  higher of two speeds: the value in MIN_SPEED or 5 steps/s.
  //  The motor continues to run at this speed until a rising edge
  //  is detected on the switch input, then a hard stop is performed
  //  and the ABS_POS register is either COPY-ed into MARK or RESET to
  //  0, depending on whether RESET or COPY was passed to the function
  //  for act.
  dspin_Xfer(RELEASE_SW | act | dir);
}

void dspin_goHome()
{
  // GoHome is equivalent to GoTo(0), but requires less time to send.
  //  Note that no direction is provided; motion occurs through shortest
  //  path. If a direction is required, use GoTo_DIR().
  dspin_Xfer(GO_HOME);
}

void dspin_goMark()
{
  // GoMark is equivalent to GoTo(MARK), but requires less time to send.
  //  Note that no direction is provided; motion occurs through shortest
  //  path. If a direction is required, use GoTo_DIR().
  dspin_Xfer(GO_MARK);
}


void dspin_setMarkByValue(uint32_t value)
{
  dspin_Xfer(MARK);

  if (value > 0x3FFFFF)
  {
    value = 0x3FFFFF;
  }

  if (value < -0x3FFFFF)
  {
    value = -0x3FFFFF;
  }

  dspin_Xfer((uint8_t)(value >> 16));
  dspin_Xfer((uint8_t)(value >> 8));
  dspin_Xfer((uint8_t)(value));
}


void dspin_setMark()
{
  uint32_t value = dspin_getPos();
  dspin_Xfer(MARK);

  if (value > 0x3FFFFF)
  {
    value = 0x3FFFFF;
  }

  if (value < -0x3FFFFF)
  {
    value = -0x3FFFFF;
  }

  dspin_Xfer((uint8_t)(value >> 16));
  dspin_Xfer((uint8_t)(value >> 8));
  dspin_Xfer((uint8_t)(value));
}

void dspin_setAsHome()
{
  // Sets the ABS_POS register to 0, effectively declaring the current
  //  position to be "HOME".
  dspin_Xfer(RESET_POS);
}

void dspin_resetDev()
{
  // Reset device to power up conditions. Equivalent to toggling the STBY
  //  pin or cycling power.
  dspin_Xfer(RESET_DEVICE);
}

void dspin_softStop()
{
  // Bring the motor to a halt using the deceleration curve.
  dspin_Xfer(SOFT_STOP);
}

void dspin_hardStop()
{
  // Stop the motor right away. No deceleration.
  dspin_Xfer(HARD_STOP);
}

void dspin_softFree()
{
  // Decelerate the motor and disengage
  dspin_Xfer(SOFT_HIZ);
}

void dspin_free()
{
  // disengage the motor immediately with no deceleration.
  dspin_Xfer(HARD_HIZ);
}

uint32_t dspin_getStatus()
{
  // Fetch and return the 16-bit value in the STATUS register. Resets
  //  any warning flags and exits any error states. Using GetParam()
  //  to read STATUS does not clear these values.
  uint32_t temp = 0;
  dspin_Xfer(GET_STATUS);
  temp = dspin_Xfer(0) << 8;
  temp |= dspin_Xfer(0);
  return temp;
}

uint32_t dspin_AccCalc(float stepsPerSecPerSec)
{
  // The value in the ACC register is [(steps/s/s)*(tick^2)]/(2^-40) where tick is
  //  250ns (datasheet value)- 0x08A on boot.
  // Multiply desired steps/s/s by .137438 to get an appropriate value for this register.
  // This is a 12-bit value, so we need to make sure the value is at or below 0xFFF.
  float temp = stepsPerSecPerSec * 0.137438;

  if ((uint32_t)(temp) > 0x00000FFF)
  {
    return 0x00000FFF;
  }
  else
  {
    return (uint32_t)(temp);
  }
}


uint32_t dspin_DecCalc(float stepsPerSecPerSec)
{
  // The calculation for DEC is the same as for ACC. Value is 0x08A on boot.
  // This is a 12-bit value, so we need to make sure the value is at or below 0xFFF.
  float temp = stepsPerSecPerSec * 0.137438;

  if ((uint32_t)(temp) > 0x00000FFF)
  {
    return 0x00000FFF;
  }
  else
  {
    return (uint32_t)(temp);
  }
}

uint32_t dspin_MaxSpdCalc(float stepsPerSec)
{
  // The value in the MAX_SPD register is [(steps/s)*(tick)]/(2^-18) where tick is
  //  250ns (datasheet value)- 0x041 on boot.
  // Multiply desired steps/s by .065536 to get an appropriate value for this register
  // This is a 10-bit value, so we need to make sure it remains at or below 0x3FF
  float temp = stepsPerSec * .065536;

  if ((uint32_t)(temp) > 0x000003FF)
  {
    return 0x000003FF;
  }
  else
  {
    return (uint32_t)(temp);
  }
}

uint32_t dspin_MinSpdCalc(float stepsPerSec)
{
  // The value in the MIN_SPD register is [(steps/s)*(tick)]/(2^-24) where tick is
  //  250ns (datasheet value)- 0x000 on boot.
  // Multiply desired steps/s by 4.1943 to get an appropriate value for this register
  // This is a 12-bit value, so we need to make sure the value is at or below 0xFFF.
  float temp = stepsPerSec * 4.1943;

  if ((uint32_t)(temp) > 0x00000FFF)
  {
    return 0x00000FFF;
  }
  else
  {
    return (uint32_t)(temp);
  }
}

uint32_t dspin_FSCalc(float stepsPerSec)
{
  // The value in the FS_SPD register is ([(steps/s)*(tick)]/(2^-18))-0.5 where tick is
  //  250ns (datasheet value)- 0x027 on boot.
  // Multiply desired steps/s by .065536 and subtract .5 to get an appropriate value for this register
  // This is a 10-bit value, so we need to make sure the value is at or below 0x3FF.
  float temp = (stepsPerSec * .065536) - .5;

  if ((uint32_t)(temp) > 0x000003FF)
  {
    return 0x000003FF;
  }
  else
  {
    return (uint32_t)(temp);
  }
}

uint32_t dspin_IntSpdCalc(float stepsPerSec)
{
  // The value in the INT_SPD register is [(steps/s)*(tick)]/(2^-24) where tick is
  //  250ns (datasheet value)- 0x408 on boot.
  // Multiply desired steps/s by 4.1943 to get an appropriate value for this register
  // This is a 14-bit value, so we need to make sure the value is at or below 0x3FFF.
  float temp = stepsPerSec * 4.1943;

  if ((uint32_t)(temp) > 0x00003FFF)
  {
    return 0x00003FFF;
  }
  else
  {
    return (uint32_t)(temp);
  }
}

uint32_t dspin_SpdCalc(float stepsPerSec)
{
  // When issuing RUN command, the 20-bit speed is [(steps/s)*(tick)]/(2^-28) where tick is
  //  250ns (datasheet value).
  // Multiply desired steps/s by 67.106 to get an appropriate value for this register
  // This is a 20-bit value, so we need to make sure the value is at or below 0xFFFFF.
  float temp = stepsPerSec * 67.106;

  if ((uint32_t)(temp) > 0x000FFFFF)
  {
    return 0x000FFFFF;
  }
  else
  {
    return (uint32_t) temp;
  }
}

uint32_t dspin_Param(uint32_t value, uint8_t bit_len)
{
  // Generalization of the subsections of the register read/write functionality.
  //  We want the end user to just write the value without worrying about length,
  //  so we pass a bit length dspin_Parameter from the calling function.
  uint32_t ret_val = 0;      // We'll return this to generalize this function
  //  for both read and write of registers.
  uint8_t byte_len = bit_len / 8;    // How many BYTES do we have?

  if (bit_len % 8 > 0)
  {
    byte_len++;  // Make sure not to lose any partial uint8_t values.
  }

  // Let's make sure our value has no spurious bits set, and if the value was too
  //  high, max it out.
  uint32_t mask = 0xffffffff >> (32 - bit_len);

  if (value > mask)
  {
    value = mask;
  }

  // The following three if statements handle the various possible uint8_t length
  //  transfers- it'll be no less than 1 but no more than 3 bytes of data.
  // dspin_Xfer() sends a uint8_t out through SPI and returns a uint8_t received
  //  over SPI- when calling it, we typecast a shifted version of the masked
  //  value, then we shift the received value back by the same amount and
  //  store it until return time.
  if (byte_len == 3)
  {
    ret_val |= (uint32_t) dspin_Xfer((uint8_t)(value >> 16)) << 16;
    ////Serial.println(ret_val, HEX);
  }

  if (byte_len >= 2)
  {
    ret_val |= dspin_Xfer((uint8_t)(value >> 8)) << 8;
    ////Serial.println(ret_val, HEX);
  }

  if (byte_len >= 1)
  {
    ret_val |= dspin_Xfer((uint8_t) value);
    ////Serial.println(ret_val, HEX);
  }

  // Return the received values. Mask off any unnecessary bits, just for
  //  the sake of thoroughness- we don't EXPECT to see anything outside
  //  the bit length range but better to be safe than sorry.
  return (ret_val & mask);
}

uint8_t dspin_Xfer(uint8_t data)
{
  // This simple function shifts a uint8_t out over SPI and receives a uint8_t over
  //  SPI_ Unusually for SPI devices, the dSPIN requires a toggling of the
  //  CS (slaveSelect) pin after each uint8_t sent. That makes this function
  //  a bit more reasonable, because we can include more functionality in it.
  uint8_t data_in;
  spi_select(g_device_index);
  data_in = spi_transmit(data);
  return data_in;
}


void dspin_SetParam(uint8_t param, uint32_t value)
{
  dspin_Xfer(SET_PARAM | param);
  dspin_ParamHandler(param, value);
}

uint32_t dspin_GetParam(uint8_t param)
{
  // Realize the "get dspin_Parameter" function, to read from the various registers in
  //  the dSPIN chip.
  dspin_Xfer(GET_PARAM | param);
  return dspin_ParamHandler(param, 0);
}

uint32_t dspin_convert(uint32_t val)
{
  //convert 22bit 2s comp to signed uint32_t
  uint32_t MSB = val >> 21;
  val = val << 11;
  val = val >> 11;

  if (MSB == 1)
  {
    val = val | 0xffe00000; //0b1111 1111 1110 0000 0000 0000 0000 0000;
  }

  return val;
}

uint32_t dspin_ParamHandler(uint8_t param, uint32_t value)
{
  // Much of the functionality between "get dspin_Parameter" and "set dspin_Parameter" is
  //  very similar, so we deal with that by putting all of it in one function
  //  here to save memory space and simplify the program.
  uint32_t ret_val = 0;   // This is a temp for the value to return.

  // This switch structure handles the appropriate action for each register.
  //  This is necessary since not all registers are of the same length, either
  //  bit-wise or byte-wise, so we want to make sure we mask out any spurious
  //  bits and do the right number of transfers. That is handled by the dSPIN_Param()
  //  function, in most cases, but for 1-uint8_t or smaller transfers, we call
  //  dspin_Xfer() directly.
  switch (param)
  {
      // ABS_POS is the current absolute offset from home. It is a 22 bit number expressed
      //  in two's complement. At power up, this value is 0. It cannot be written when
      //  the motor is running, but at any other time, it can be updated to change the
      //  interpreted position of the motor.
    case ABS_POS:
      ret_val = dspin_Param(value, 22);
      break;

      // EL_POS is the current electrical position in the step generation cycle. It can
      //  be set when the motor is not in motion. Value is 0 on power up.
    case EL_POS:
      ret_val = dspin_Param(value, 9);
      break;

      // MARK is a second position other than 0 that the motor can be told to go to. As
      //  with ABS_POS, it is 22-bit two's complement. Value is 0 on power up.
    case MARK:
      ret_val = dspin_Param(value, 22);
      break;

      // SPEED contains information about the current speed. It is read-only. It does
      //  NOT provide direction information.
    case SPEED:
      ret_val = dspin_Param(0, 20);
      break;

      // ACC and DEC set the acceleration and deceleration rates. Set ACC to 0xFFF
      //  to get infinite acceleration/decelaeration- there is no way to get infinite
      //  deceleration w/o infinite acceleration (except the HARD STOP command).
      //  Cannot be written while motor is running. Both default to 0x08A on power up.
      // AccCalc() and DecCalc() functions exist to convert steps/s/s values into
      //  12-bit values for these two registers.
    case ACC:
      ret_val = dspin_Param(value, 12);
      break;

    case DEC:
      ret_val = dspin_Param(value, 12);
      break;

      // MAX_SPEED is just what it says- any command which attempts to set the speed
      //  of the motor above this value will simply cause the motor to turn at this
      //  speed. Value is 0x041 on power up.
      // dspin_MaxSpdCalc() function exists to convert steps/s value into a 10-bit value
      //  for this register.
    case MAX_SPEED:
      ret_val = dspin_Param(value, 10);
      break;

      // MIN_SPEED controls two things- the activation of the low-speed optimization
      //  feature and the lowest speed the motor will be allowed to operate at. LSPD_OPT
      //  is the 13th bit, and when it is set, the minimum allowed speed is automatically
      //  set to zero. This value is 0 on startup.
      // MinSpdCalc() function exists to convert steps/s value into a 12-bit value for this
      //  register. SetLowSpeedOpt() function exists to enable/disable the optimization feature.
    case MIN_SPEED:
      ret_val = dspin_Param(value, 12);
      break;

      // FS_SPD register contains a threshold value above which microstepping is disabled
      //  and the dSPIN operates in full-step mode. Defaults to 0x027 on power up.
      // FSCalc() function exists to convert steps/s value into 10-bit integer for this
      //  register.
    case FS_SPD:
      ret_val = dspin_Param(value, 10);
      break;

      // KVAL is the maximum voltage of the PWM outputs. These 8-bit values are ratiometric
      //  representations: 255 for full output voltage, 128 for half, etc. Default is 0x29.
      // The implications of different KVAL settings is too complex to dig into here, but
      //  it will usually work to max the value for RUN, ACC, and DEC. Maxing the value for
      //  HOLD may result in excessive power dissipation when the motor is not running.
    case KVAL_HOLD:
      ret_val = dspin_Xfer((uint8_t) value);
      break;

    case KVAL_RUN:
      ret_val = dspin_Xfer((uint8_t) value);
      break;

    case KVAL_ACC:
      ret_val = dspin_Xfer((uint8_t) value);
      break;

    case KVAL_DEC:
      ret_val = dspin_Xfer((uint8_t) value);
      break;

      // INT_SPD, ST_SLP, FN_SLP_ACC and FN_SLP_DEC are all related to the back EMF
      //  compensation functionality. Please see the datasheet for details of this
      //  function- it is too complex to discuss here. Default values seem to work
      //  well enough.
    case INT_SPD:
      ret_val = dspin_Param(value, 14);
      break;

    case ST_SLP:
      ret_val = dspin_Xfer((uint8_t) value);
      break;

    case FN_SLP_ACC:
      ret_val = dspin_Xfer((uint8_t) value);
      break;

    case FN_SLP_DEC:
      ret_val = dspin_Xfer((uint8_t) value);
      break;

      // K_THERM is motor winding thermal drift compensation. Please see the datasheet
      //  for full details on operation- the default value should be okay for most users.
    case K_THERM:
      ret_val = dspin_Xfer((uint8_t) value & 0x0F);
      break;

      // ADC_OUT is a read-only register containing the result of the ADC measurements.
      //  This is less useful than it sounds; see the datasheet for more information.
    case ADC_OUT:
      ret_val = dspin_Xfer(0);
      break;

      // Set the overcurrent threshold. Ranges from 375mA to 6A in steps of 375mA.
      //  A set of defined constants is provided for the user's convenience. Default
      //  value is 3.375A- 0x08. This is a 4-bit value.
    case OCD_TH:
      ret_val = dspin_Xfer((uint8_t) value & 0x0F);
      break;

      // Stall current threshold. Defaults to 0x40, or 2.03A. Value is from 31.25mA to
      //  4A in 31.25mA steps. This is a 7-bit value.
    case STALL_TH:
      ret_val = dspin_Xfer((uint8_t) value & 0x7F);
      break;

      // STEP_MODE controls the microstepping settings, as well as the generation of an
      //  output signal from the dSPIN. Bits 2:0 control the number of microsteps per
      //  step the part will generate. Bit 7 controls whether the BUSY/SYNC pin outputs
      //  a BUSY signal or a step synchronization signal. Bits 6:4 control the frequency
      //  of the output signal relative to the full-step frequency; see datasheet for
      //  that relationship as it is too complex to reproduce here.
      // Most likely, only the microsteps per step value will be needed; there is a set
      //  of constants provided for ease of use of these values.
    case STEP_MODE:
      //DEBUG_PRINT("setparam step_mode %i\n", value);
      ret_val = dspin_Xfer((uint8_t) value);
      break;

      // ALARM_EN controls which alarms will cause the FLAG pin to fall. A set of constants
      //  is provided to make this easy to interpret. By default, ALL alarms will trigger the
      //  FLAG pin.
    case ALARM_EN:
      ret_val = dspin_Xfer((uint8_t) value);
      break;

      // CONFIG contains some assorted configuration bits and fields. A fairly comprehensive
      //  set of reasonably self-explanatory constants is provided, but users should refer
      //  to the datasheet before modifying the contents of this register to be certain they
      //  understand the implications of their modifications. Value on boot is 0x2E88; this
      //  can be a useful way to verify proper start up and operation of the dSPIN chip.
    case CONFIG:
      ret_val = dspin_Param(value, 16);
      break;

      // STATUS contains read-only information about the current condition of the chip. A
      //  comprehensive set of constants for masking and testing this register is provided, but
      //  users should refer to the datasheet to ensure that they fully understand each one of
      //  the bits in the register.
    case STATUS:  // STATUS is a read-only register
      ret_val = dspin_Param(0, 16);
      break;

    default:
      ret_val = dspin_Xfer((uint8_t)(value));
      break;
  }

  return ret_val;
}
