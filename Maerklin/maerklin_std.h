#ifndef MAERKLIN_MAIN_H
#define MAERKLIN_MAIN_H


#define OUTPUT 0
#define INPUT 1
#define INPUT_PULLUP 2
#define OFF 0
#define ON 1
#define LEFT 0
#define RIGHT 1

#define NO_ACTION 255

#define SENSOR_COUNT 12
#define AREA_COUNT 10
#define SPEED_COUNT 2
#define SWITCH_COUNT 6

#define SHIFT_REG_COUNT 3

#define EVT_NONE 0
#define EVT_AREA 1
#define EVT_SPEED 2
#define EVT_SWITCH 3

#define EVT_QUEUE_LEN 32

#define SPEED_WAIT_FOR_1_OFF 3
#define SPEED_WAIT_FOR_TURN_OFF 2
#define SPEED_WAIT_FOR_2_ON 1
#define SPEED_END 0

#define SWITCH_ACTIV_PIN_2 51
#define SWITCH_ACTIV_PIN_1 54
#define SWITCH_ACTIV_PIN_0 55
#define SPEED_TURN_PIN 32

#define SHIFT_DATA_PIN 31
#define SHIFT_LATCH_PIN 16
#define SHIFT_PIN 17

#define SWITCH_WAIT 1000
#define SPEED_CHANGE_WAIT 30

#include <avr/io.h>
#include <avr/interrupt.h>

static const uint8_t sensor_pins[] = {
	8, 9, 11, 12, 13, 14, 15, 26, 27, 28, 29, 30
};

static const uint8_t speed_pins[] = {
	33, 34
};

static const uint8_t area_pins[] = {
	35, 36, 37, 38, 39, 40, 41, 42, 43, 44
};

static const uint8_t switch_pins[] = {
	45, 46, 47, 48, 49, 50
};


/*
 *	Holds the required information for one event
 *	Should be initialized through the corresponding
 *	create_event()-function
*/

typedef struct event_t {
	unsigned int type : 2;
	unsigned int activated : 1;
	unsigned int new_state : 1;
	unsigned int own_id : 4;
	unsigned int sensor_id : 4;
	uint16_t wait;
	uint16_t wait_point;
} event_t;


/*
 * Initializes the library so that it is ready for accepting any events
 * as well as any calls to the several xxx_pin()-functions
*/

void init_maerklin_std();


/*
 *	Checks on all the sensors and executes any events attached to them
 *	and enqueues all of them which have a delay
*/

uint8_t control_cycle();


/*
 * Runs the actions specified by the parameters in the given event_t-struct
*/

void _exec_event(event_t *evt);


/*
 * Populates an event_t-struct with the correct values
*/

uint8_t create_event(event_t* evt, uint8_t type, uint8_t own_id, uint8_t new_state, uint8_t sensor_id, uint16_t wait);


/*
 *	Attaches an event so that it will be executed as soon as the conditions
 *	specified in the passed event_t-struct are true
*/

uint8_t attach_event(event_t* evt);


/*
 *	Configures a pin to take a specific type
 *	Please use the given Macros (OUTPUT, INPUT and INPUT_PULLUP)
*/

void config_pin(uint8_t pin, uint8_t type);


/*
 *	Reads and returns the state of the passed pin number
*/

uint8_t read_pin(uint8_t pin);


/*
 *	Writes a given state to a pin
 *	Please use the given macros (ON and OFF) or 1 and 0 respectively
*/

void write_pin(uint8_t pin, uint8_t new_state);


/*
 *	Shifts out all of the array contents destined for the shift registers
*/

void _shift_out();


/*
 *	Lights up an LED connected to pin 1, if the specified int is a valid error code
 *	(Used for debug)
*/

void led_show_err(uint8_t err_code);


/*
 *	Lights up an LED connected to pin 2, if the specified int is not null
*/

void led_show_success(uint8_t enable);


/*
 *	Lights up an LED connected to pin 3, if the specified int is not null
*/

void led_show_waiting(uint8_t enable);

#endif
