#ifndef THERMISTOR_H
#define THERMISTOR_H

#include <math.h>

/**
 * Data needed to calculate the temperature from a thermistor reading.
 *
 * Assumes the following potential divider.
 *      ,-----[ r ]----,----[ rt ]----,
 *      |              |              |
 *     vref         signal            0v
 * Where r is a resistor of resistance r ohms and rt a thermistor with:
 *   r0 = resistance at t0 in ohms
 *   b  = Beta value
 *   t0 = Refrence temperature in Kelvin
 *
 */
typedef struct {
	double r;   // Resistor value (ohms)
	double r0;  // Resistance of thermistor at t0 (ohms)
	double b_;  // Reciprocal of beta value of thermistor
	double t0_; // Reciprocal of Refrence temperature in kelvin
} thermistor_t;


/**
 * Initialise a thermistor struct.
 *
 * @param r  Resistor value (ohms)
 * @param r0 Resistance of thermistor at t0 (ohms)
 * @param b  Beta value of thermistor
 * @param t0 Refrence temperature in kelvin
 */
void thermistor_init(thermistor_t *thermistor,
                     double r, double r0, double b, double t0);


/**
 * Convert a reading into a temperature in kelvin.
 *
 * @param v_vref The voltage read over the refrence voltage.
 */
double thermistor_convert(thermistor_t *thermistor, double v_vref);



#endif
