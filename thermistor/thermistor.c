#include "thermistor.h"

void
thermistor_init(thermistor_t *thermistor,
                double r, double r0, double b, double t0)
{
	thermistor->r   = r;
	thermistor->r0  = r0;
	thermistor->b_  = 1.0 / b;
	thermistor->t0_ = 1.0 / t0;
}

double
thermistor_convert(thermistor_t *thermistor, double v_vref)
{
	// Convert the relative voltage to a resistance in ohms, rt
	//    rt = (v_vref * r) / (1 - v_vref)
	// From:
	//    v_vref = rt / (r + rt)
	double rt = (v_vref * thermistor->r) / (1.0 - v_vref);
	
	// Calculate the reciprocal of the temperature using:
	// 1/t = 1/t0 + 1/b * ln(rt / r0)
	double t_ = thermistor->t0_ + (thermistor->b_ * log(rt / thermistor->r0));
	
	return 1.0 / t_;
}
