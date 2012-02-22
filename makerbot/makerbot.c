#include "makerbot.h"

static makerbot_t makerbot;


void
makerbot_init(void)
{
	int i;
	
	// Initialise the queue of commands to the printer
	makerbot.command_queue = xQueueCreate(MAKERBOT_COMMAND_QUEUE_LENGTH,
	                                      sizeof(makerbot_command_t));
	vSemaphoreCreateBinary(makerbot.command_queue_lock);
	
	// Check the command_queue was successfully created
	// XXX: Do something better than infini-looping
	if (makerbot.command_queue == 0)
		while (true)
			;
	
	#if MAKERBOT_NUM_AXES != 3
		#error "makerbot_init can't handle MAKERBOT_NUM_AXES many axes"
	#endif
	
	// Set up stepper numbers
	makerbot.axes[0].stepper_num = 0;
	makerbot.axes[1].stepper_num = 1;
	makerbot.axes[2].stepper_num = 2;
	
	// Set step sizes
	// XXX: Load from configuration
	makerbot.axes[0].steps_per_mm = STEPPER_0_STEPS_PER_MM;
	makerbot.axes[1].steps_per_mm = STEPPER_1_STEPS_PER_MM;
	makerbot.axes[2].steps_per_mm = STEPPER_2_STEPS_PER_MM;
	
	// Start assuming we're at the origin
	makerbot.axes[0].position = 0;
	makerbot.axes[1].position = 0;
	makerbot.axes[2].position = 0;
	
	// Set up the stepper motors and disable them
	stepper_init_motor(0, PIN_STEPPER_0_NEN, PIN_STEPPER_0_DIR, PIN_STEPPER_0_STEP);
	stepper_init_motor(1, PIN_STEPPER_1_NEN, PIN_STEPPER_1_DIR, PIN_STEPPER_1_STEP);
	stepper_init_motor(2, PIN_STEPPER_2_NEN, PIN_STEPPER_2_DIR, PIN_STEPPER_2_STEP);
	
	#if MAKERBOT_NUM_HEATERS != 2
		#error "makerbot_init can't handle MAKERBOT_NUM_HEATERS many heaters"
	#endif
	
	// Set up heater pins
	makerbot.heaters[0].heater_pin     = PIN_HEATER_EXTRUDER;
	makerbot.heaters[0].led_pin        = PIN_LED_EXTRUDER;
	makerbot.heaters[0].thermistor_pin = PIN_THERMISTOR_EXTRUDER;
	makerbot.heaters[1].heater_pin     = PIN_HEATER_PLATFORM;
	makerbot.heaters[1].led_pin        = PIN_LED_PLATFORM;
	makerbot.heaters[1].thermistor_pin = PIN_THERMISTOR_PLATFORM;
	
	for (i = 0; i < MAKERBOT_NUM_HEATERS; i++) {
		// Set up ports
		gpio_set_mode(makerbot.heaters[i].heater_pin, GPIO_OUTPUT);
		gpio_set_mode(makerbot.heaters[i].led_pin, GPIO_OUTPUT);
		analog_in_set_mode(makerbot.heaters[i].thermistor_pin);
		
		// Start with the heater disabled
		gpio_write(makerbot.heaters[i].heater_pin, GPIO_LOW);
		gpio_write(makerbot.heaters[i].led_pin, GPIO_LOW);
		
		makerbot.heaters[i].reached = false;
		makerbot.heaters[i].set_point = 0.0;
	}
	
	// A semaphore for when checking if heaters have reached temperature
	vSemaphoreCreateBinary(makerbot.heaters_reached);
	
	// Setup the semaphore that indicates when the heaters have heated
	vSemaphoreCreateBinary(makerbot.heaters_on_reach);
	xSemaphoreTake(makerbot.heaters_on_reach, portMAX_DELAY);
	
	// Set up thermistors
	thermistor_init(&(makerbot.heaters[0].thermistor),
	                THERMISTOR_EXTRUDER_R, THERMISTOR_EXTRUDER_R0,
	                THERMISTOR_EXTRUDER_B, THERMISTOR_EXTRUDER_T0);
	thermistor_init(&(makerbot.heaters[1].thermistor),
	                THERMISTOR_PLATFORM_R, THERMISTOR_PLATFORM_R0,
	                THERMISTOR_PLATFORM_B, THERMISTOR_PLATFORM_T0);
	
	// Set the refrence voltages in the potential divider
	makerbot.heaters[0].v_vref = THERMISTOR_EXTRUDER_V_VREF;
	makerbot.heaters[1].v_vref = THERMISTOR_PLATFORM_V_VREF;
	
	// Set up the PID controllers (with 0 previous error)
	// XXX: Load from system configuration
	pid_init(&(makerbot.heaters[0].pid), THERMISTOR_EXTRUDER_P,
	         THERMISTOR_EXTRUDER_I, THERMISTOR_EXTRUDER_D, 0);
	pid_init(&(makerbot.heaters[1].pid), THERMISTOR_PLATFORM_P,
	         THERMISTOR_PLATFORM_I, THERMISTOR_PLATFORM_D, 0);
	
	// Set other pins
	makerbot.extruder = PIN_EXTRUDER;
	makerbot.platform = PIN_PLATFORM;
	makerbot.psu      = PIN_POWER_EN;
	makerbot.psu_ok   = PIN_POWER_OK;
	
	// Set up ports
	gpio_set_mode(makerbot.extruder, GPIO_OUTPUT);
	gpio_set_mode(makerbot.platform, GPIO_OUTPUT);
	gpio_set_mode(makerbot.psu,      GPIO_OUTPUT);
	gpio_set_mode(makerbot.psu_ok,   GPIO_INPUT);
	
	// Start with everything off
	gpio_write(makerbot.extruder, GPIO_LOW);
	gpio_write(makerbot.platform, GPIO_LOW);
	gpio_write(makerbot.psu,      GPIO_LOW);
}


// XXX
#include "network_debug.h"
static int XXX_count = 0;


void
makerbot_main_task(void *pvParameters)
{
	int i;
	bool moved;
	
	for (;;) {
		// Get the next command
		makerbot_command_t cmd;
		while(!xQueueReceive(makerbot.command_queue, &cmd, portMAX_DELAY))
			;
		
		// Timer for PSU operations
		portTickType last_update = xTaskGetTickCount();
	
		switch (cmd.instr) {
			case MAKERBOT_NOP:
				// Do nothing
				break;
			
			case MAKERBOT_SET_ORIGIN:
				// Do nothing
				break;
			
			case MAKERBOT_MOVE_TO:
				moved = false;
				
				for (i = 0; i < MAKERBOT_NUM_AXES; i++) {
					if (cmd.arg.move_to.step_periods[i] > 0) {
						moved = true;
						stepper_set_action(makerbot.axes[i].stepper_num,
						                   cmd.arg.move_to.directions[i],
						                   cmd.arg.move_to.num_steps[i],
						                   cmd.arg.move_to.step_periods[i]);
					}
				}
				sprintf(network_debug_str(), "Wait started %d (%d, %d, %d)\n",
				        XXX_count,
				        (int)(cmd.arg.move_to.num_steps[0] * 100.0),
				        (int)(cmd.arg.move_to.num_steps[1] * 100.0),
				        (int)(cmd.arg.move_to.num_steps[2] * 100.0));
				if (moved)
					stepper_wait_until_idle();
				sprintf(network_debug_str(), "Wait done %d (%d, %d, %d)\n",
				        XXX_count++,
				        (int)(cmd.arg.move_to.num_steps[0] * 100.0),
				        (int)(cmd.arg.move_to.num_steps[1] * 100.0),
				        (int)(cmd.arg.move_to.num_steps[2] * 100.0));
				break;
			
			case MAKERBOT_SET_TEMPERATURE:
				makerbot.heaters[cmd.arg.set_temperature.heater_num].set_point
					= cmd.arg.set_temperature.temperature;
				makerbot.heaters[cmd.arg.set_temperature.heater_num].reached = false;
				break;
			
			case MAKERBOT_WAIT_HEATERS:
				// Prevent the haters from being updated while we're checking
				xSemaphoreTake(makerbot.heaters_reached, portMAX_DELAY);
				
				for (i = 0; i < MAKERBOT_NUM_HEATERS; i++) {
					// If a heater hasn't warmed up, wait until all heaters are warm
					if (!_makerbot_temperature_reached(i)) {
						// Clear the semaphore (if its set)
						xSemaphoreTake(makerbot.heaters_on_reach, ( portTickType ) 0);
						
						// Allow the heater routine checking to run
						xSemaphoreGive(makerbot.heaters_reached);
						
						// Wait for the heaters to warm up
						xSemaphoreTake(makerbot.heaters_on_reach, portMAX_DELAY);
						break;
					}
				}
				
				// Allow the heater routine checking to run
				xSemaphoreGive(makerbot.heaters_reached);
				break;
			
			case MAKERBOT_SLEEP:
				vTaskDelayUntil(&last_update, cmd.arg.sleep.duration/portTICK_RATE_MS);
				break;
			
			case MAKERBOT_SET_EXTRUDER:
				gpio_write(makerbot.extruder, cmd.arg.set_extruder.enabled);
				break;
			
			case MAKERBOT_SET_PLATFORM:
				gpio_write(makerbot.platform, cmd.arg.set_platform.enabled);
				break;
			
			case MAKERBOT_SET_POWER:
				gpio_write(makerbot.psu, cmd.arg.set_power.enabled);
				while (!gpio_read(makerbot.psu_ok))
					vTaskDelayUntil(&last_update, MAKERBOT_PSU_DELAY/portTICK_RATE_MS);
				break;
			
			case MAKERBOT_SET_AXES_ENABLED:
				for (i = 0; i < MAKERBOT_NUM_AXES; i++)
					stepper_set_enabled(makerbot.axes[i].stepper_num,
					                    cmd.arg.set_axes_enabled.enabled);
				break;
			
			default:
				// XXX: Do something a bit more sensible than this
				while (true)
					;
				break;
		}
	}
}


void
makerbot_pid_task(void *pvParameters)
{
	portTickType last_update = xTaskGetTickCount();
	portTickType delay       = MAKERBOT_PERIOD;
	
	// Mainloop
	for (;;) {
		_makerbot_pid((double)MAKERBOT_PERIOD_MS);
		
		watchdog_feed();
		
		vTaskDelayUntil(&last_update, delay);
	}
}


void
_makerbot_pid(double dt)
{
	int i;
	
	xSemaphoreTake(makerbot.heaters_reached, portMAX_DELAY);
	
	// Note if the heaters have warmed up previously
	bool temperature_reached = true;
	for (i = 0; i < MAKERBOT_NUM_HEATERS; i++)
		temperature_reached &= _makerbot_temperature_reached(i);
	
	// Run the PID routine for all heaters
	for (i = 0; i < MAKERBOT_NUM_HEATERS; i++) {
		double temp_c = makerbot_get_temperature(i);
		double control = pid_update(&(makerbot.heaters[i].pid),
		                            makerbot.heaters[i].set_point,
		                            temp_c,
		                            dt);
		bool heater_on = (control > 0.0) && _makerbot_get_power();
		bool reached = makerbot.heaters[i].set_point <= temp_c;
		
		// Turn on heater (and LED) if needed
		gpio_write(makerbot.heaters[i].heater_pin, heater_on);
		gpio_write(makerbot.heaters[i].led_pin, heater_on);
		
		// Record that the temperature has been reached
		makerbot.heaters[i].reached |= reached;
	}
	
	// Raise event if all heaters just heated up
	if (!temperature_reached) {
		bool now_reached = true;
		for (i = 0; i < MAKERBOT_NUM_HEATERS; i++)
			now_reached &= _makerbot_temperature_reached(i);
		
		if (now_reached)
			xSemaphoreGive(makerbot.heaters_on_reach);
	}
	
	xSemaphoreGive(makerbot.heaters_reached);
}


void
makerbot_reset(void)
{
	int i;
	
	// Empty the queue
	xSemaphoreTake(makerbot.command_queue_lock, portMAX_DELAY);
	
	makerbot_command_t cmd_buffer;
	while (uxQueueMessagesWaiting(makerbot.command_queue) != 0)
		xQueueReceive(makerbot.command_queue, &cmd_buffer, portMAX_DELAY);
	
	xSemaphoreGive(makerbot.command_queue_lock);
	
	// Power off motors & stop any movements (which releases the semaphore
	// indicating motion has finished)
	for (i = 0; i < MAKERBOT_NUM_AXES; i++)
		stepper_set_enabled(makerbot.axes[i].stepper_num, false);
	
	// Power off heaters
	for (i = 0; i < MAKERBOT_NUM_HEATERS; i++) {
		gpio_write(makerbot.heaters[i].heater_pin, GPIO_LOW);
		gpio_write(makerbot.heaters[i].led_pin, GPIO_LOW);
		makerbot.heaters[i].set_point = 0.0;
		makerbot.heaters[i].reached = false;
	}
	
	// Release the temperature wait
	xSemaphoreGive(makerbot.heaters_on_reach);
	
	// Turn off the extruder and platform
	makerbot_set_extruder(false);
	makerbot_set_platform(false);
}


void
makerbot_set_origin(double offset_mm[MAKERBOT_NUM_AXES])
{
	int i;
	
	for (i = 0; i < MAKERBOT_NUM_AXES; i++)
		makerbot.axes[i].position = offset_mm[i] * makerbot.axes[i].steps_per_mm;
}

void
makerbot_move_to(double pos_mm[MAKERBOT_NUM_AXES], double speed_mm_s)
{
	int i;
	
	makerbot_command_t cmd;
	cmd.instr = MAKERBOT_MOVE_TO;
	
	// Calculate the distance the head will travel (pythagoras)
	double sum_mm = 0.0;
	for (i = 0; i < MAKERBOT_NUM_AXES; i++) {
		double delta = pos_mm[i] - (((double)makerbot.axes[i].position)
		                            / makerbot.axes[i].steps_per_mm);
		sum_mm += delta * delta;
	}
	double distance_mm = sqrt(sum_mm);
	
	// Do nothing for 0-length line
	if (distance_mm == 0.0)
		return;
	
	// Calculate the time this should take
	double time_s = distance_mm / speed_mm_s;
	long time_ticks = (long)(time_s * (double)STEPPER_TIMER_HZ);
	
	// Set each motor running
	for (i = 0; i < MAKERBOT_NUM_AXES; i++) {
		// How far are we travelling
		int num_steps = (int)(pos_mm[i] * makerbot.axes[i].steps_per_mm)
		                - makerbot.axes[i].position;
		
		// Do nothing if no movement required
		if (num_steps == 0) {
			cmd.arg.move_to.num_steps[i] = 0;
			cmd.arg.move_to.step_periods[i] = 0;
			continue;
		}
		
		// How long should steps take to make the move last time_ticks
		int step_period = time_ticks / abs(num_steps);
		
		stepper_dir_t direction = (num_steps >= 0) ? STEPPER_FORWARD
		                                           : STEPPER_BACKWARD;
		
		// Record the action required
		cmd.arg.move_to.directions[i]   = direction;
		cmd.arg.move_to.num_steps[i]    = abs(num_steps);
		cmd.arg.move_to.step_periods[i] = step_period;
		
		// Update position
		makerbot.axes[i].position += num_steps;
	}
	
	xSemaphoreTake(makerbot.command_queue_lock, portMAX_DELAY);
	xQueueSend(makerbot.command_queue, &cmd, portMAX_DELAY);
	xSemaphoreGive(makerbot.command_queue_lock);
}


double
makerbot_get_position(int axis)
{
	return ((double)makerbot.axes[axis].position) / makerbot.axes[axis].steps_per_mm;
}


void
makerbot_set_temperature(int heater_num, double temperature)
{
	makerbot_command_t cmd;
	cmd.instr = MAKERBOT_SET_TEMPERATURE;
	
	cmd.arg.set_temperature.heater_num  = heater_num;
	cmd.arg.set_temperature.temperature = temperature;
	
	xSemaphoreTake(makerbot.command_queue_lock, portMAX_DELAY);
	xQueueSend(makerbot.command_queue, &cmd, portMAX_DELAY);
	xSemaphoreGive(makerbot.command_queue_lock);
}


double
makerbot_get_temperature(int heater_num)
{
	int reading = analog_in_read(makerbot.heaters[heater_num].thermistor_pin);
	return thermistor_convert(&makerbot.heaters[heater_num].thermistor,
	                          ((double)reading) / 4086.0) - 273.0;
}


bool
_makerbot_temperature_reached(int heater_num)
{
	return makerbot.heaters[heater_num].reached;
}


void
makerbot_wait_heaters(void)
{
	makerbot_command_t cmd;
	cmd.instr = MAKERBOT_WAIT_HEATERS;
	
	xSemaphoreTake(makerbot.command_queue_lock, portMAX_DELAY);
	xQueueSend(makerbot.command_queue, &cmd, portMAX_DELAY);
	xSemaphoreGive(makerbot.command_queue_lock);
}

void
makerbot_sleep(int duration)
{
	makerbot_command_t cmd;
	cmd.instr = MAKERBOT_SLEEP;
	
	cmd.arg.sleep.duration = duration;
	
	xSemaphoreTake(makerbot.command_queue_lock, portMAX_DELAY);
	xQueueSend(makerbot.command_queue, &cmd, portMAX_DELAY);
	xSemaphoreGive(makerbot.command_queue_lock);
}


void
makerbot_set_extruder(bool enabled)
{
	makerbot_command_t cmd;
	cmd.instr = MAKERBOT_SET_EXTRUDER;
	
	cmd.arg.set_extruder.enabled = enabled;
	
	xSemaphoreTake(makerbot.command_queue_lock, portMAX_DELAY);
	xQueueSend(makerbot.command_queue, &cmd, portMAX_DELAY);
	xSemaphoreGive(makerbot.command_queue_lock);
}


void
makerbot_set_platform(bool enabled)
{
	makerbot_command_t cmd;
	cmd.instr = MAKERBOT_SET_PLATFORM;
	
	cmd.arg.set_platform.enabled = enabled;
	
	xSemaphoreTake(makerbot.command_queue_lock, portMAX_DELAY);
	xQueueSend(makerbot.command_queue, &cmd, portMAX_DELAY);
	xSemaphoreGive(makerbot.command_queue_lock);
}


void
makerbot_set_power(bool enabled)
{
	makerbot_command_t cmd;
	cmd.instr = MAKERBOT_SET_POWER;
	
	cmd.arg.set_power.enabled = enabled;
	
	xSemaphoreTake(makerbot.command_queue_lock, portMAX_DELAY);
	xQueueSend(makerbot.command_queue, &cmd, portMAX_DELAY);
	xSemaphoreGive(makerbot.command_queue_lock);
}

bool
_makerbot_get_power(void)
{
	return gpio_read(makerbot.psu);
}


void
makerbot_set_axes_enabled(bool enabled)
{
	makerbot_command_t cmd;
	cmd.instr = MAKERBOT_SET_AXES_ENABLED;
	
	cmd.arg.set_axes_enabled.enabled = enabled;
	
	xSemaphoreTake(makerbot.command_queue_lock, portMAX_DELAY);
	xQueueSend(makerbot.command_queue, &cmd, portMAX_DELAY);
	xSemaphoreGive(makerbot.command_queue_lock);
}
