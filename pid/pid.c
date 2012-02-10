#include "pid.h"


#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))


void
pid_init(pid_state_t *pid, double Kp, double Ki, double Kd, double previous_error)
{
	pid->Kp = Kp;
	pid->Ki = Ki;
	pid->Kd = Kd;
	
	pid->previous_error = previous_error;
	
	pid->integral = 0.0;
}


void
pid_set_Kp(pid_state_t *pid, double Kp)
{
	pid->Kp = Kp;
}


void
pid_set_Ki(pid_state_t *pid, double Ki)
{
	pid->Ki = Ki;
}


void
pid_set_Kd(pid_state_t *pid, double Kd)
{
	pid->Kd = Kd;
}


double
pid_get_Kp(pid_state_t *pid)
{
	return pid->Kp;
}


double
pid_get_Ki(pid_state_t *pid)
{
	return pid->Ki;
}


double
pid_get_Kd(pid_state_t *pid)
{
	return pid->Kd;
}


double
pid_update(pid_state_t *pid,
           double set_point,
           double process_variable,
           double dt)
{
	// Caluclate the error (P)
	double error = set_point - process_variable;
	
	// Update the integral (I)
	pid->integral += error * dt;
	pid->integral = MAX(-100.0, MIN(100.0, pid->integral));
	
	// Calculate the derivative (D)
	double derivative = (error - pid->previous_error) / dt;
	
	pid->previous_error = error;
	
	return (pid->Kp * error) + (pid->Ki * pid->integral) + (pid->Kd * derivative);
}
