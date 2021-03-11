typedef struct
{
  double proportional;
  double integral;
  double derivative;
} pidcfg_t;

typedef struct
{
  double   windup_guard;
  pidcfg_t *gain;
  double   prev_error;
  double   int_error;
  double   control;
} pidctl_t;

void pid_init(pidctl_t *pid);
void pid_zeroize(pidctl_t *pid);
void pid_update(pidctl_t *pid, double curr_error, double dt);
