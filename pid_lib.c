#include "project.h"

// http://nicisdigital.wordpress.com/2011/06/27/proportional-integral-derivative-pid-controller/

void pid_init(pidctl_t *pid)
{
  memset(pid, 0, sizeof(*pid));
}

void pid_zeroize(pidctl_t *pid)
{
  // set prev and integrated error to zero
  pid->prev_error = 0;
  pid->int_error = 0;
}

void pid_update(pidctl_t *pid, double curr_error, double dt)
{
  double diff;
  double p_term;
  double i_term;
  double d_term;
  // integration with windup guarding
  pid->int_error += (curr_error * dt);

  if (pid->int_error < - (pid->windup_guard))
  {
    pid->int_error = - (pid->windup_guard);
  }
  else if (pid->int_error > pid->windup_guard)
  {
    pid->int_error = pid->windup_guard;
  }

  // differentiation
  diff = ((curr_error - pid->prev_error) / dt);
  // scaling
  p_term = (pid->gain->proportional * curr_error);
  i_term = (pid->gain->integral     * pid->int_error);
  d_term = (pid->gain->derivative   * diff);
  // summation of terms
  pid->control = p_term + i_term + d_term;
  // save current error as previous error for next iteration
  pid->prev_error = curr_error;
}

