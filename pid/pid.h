#ifndef PID_H
#define PID_H

/**
 * A structure which keeps the state and tuning values of a PID controller.
 */
typedef struct pid {
	/* Tuning values */
	double Kp;
	double Ki;
	double Kd;
	
	/* Internal state */
	double previous_error;
	double integral;
} pid_state_t;


/**
 * Initialise a PID structure.
 */
void pid_init(pid_state_t *pid, double Kp, double Ki, double Kd, double previous_error);


/**
 * Set the PID tuning values.
 */
void pid_set_Kp(pid_state_t *pid, double Kp);
void pid_set_Ki(pid_state_t *pid, double Ki);
void pid_set_Kd(pid_state_t *pid, double Kd);

/**
 * Get the PID tuning values.
 */
double pid_get_Kp(pid_state_t *pid);
double pid_get_Ki(pid_state_t *pid);
double pid_get_Kd(pid_state_t *pid);


/**
 * Update the PID controller after some time, dt, has elapsed. Returns the
 * modifiable variable.
 *
 * @param set_point        The target value
 * @param process_variable The current actual value
 * @param dt               The time elapsed since update was last called (must
 *                         be > 0)
 */
double pid_update(pid_state_t *pid,
                  double set_point,
                  double process_variable,
                  double dt);


#endif
