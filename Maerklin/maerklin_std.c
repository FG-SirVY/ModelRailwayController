#include "maerklin_std.h"


uint8_t speed_change_state;
uint8_t switch_activ_2_on;
uint8_t switch_activ_1_on;
uint8_t switch_activ_0_on;

uint16_t speed_wait;
uint16_t switch_2_wait;
uint16_t switch_1_wait;
uint16_t switch_0_wait;

volatile uint16_t timer_100ms;
volatile uint16_t timer_1ms;
volatile uint8_t timer_1ms_in_use;

uint8_t next_speed;

event_t events[EVT_QUEUE_LEN];
uint16_t sensor_cooldown[SENSOR_COUNT];

uint8_t shifts[SHIFT_REG_COUNT];


/*
* Initialization function initializes several variables.
*/

void init_maerklin_std()
{
	TCNT0 = 0;
	TIMSK0 |= (1 << OCIE0A);
	OCR0A = 250;

	uint8_t i = 0;

	for (i = 0; i < EVT_QUEUE_LEN; ++i) {
		events[i].activated = 0;
		events[i].type = EVT_NONE;
	}

	next_speed = 1;
	speed_change_state = SPEED_END;
	switch_activ_2_on = 0;
	timer_1ms_in_use = 0;

	for (i = 0; i < SENSOR_COUNT; ++i) {
		config_pin(sensor_pins[i], INPUT_PULLUP);
	}

	for (i = 0; i < AREA_COUNT; ++i) {
		config_pin(area_pins[i], OUTPUT);
	}

	for (i = 0; i < SPEED_COUNT; ++i) {
		config_pin(speed_pins[i], OUTPUT);
	}

	for (i = 0; i < SWITCH_COUNT; ++i) {
		config_pin(switch_pins[i], OUTPUT);
	}
	
	config_pin(SWITCH_ACTIV_PIN_0, OUTPUT);
	config_pin(SWITCH_ACTIV_PIN_1, OUTPUT);
	config_pin(SWITCH_ACTIV_PIN_2, OUTPUT);
	config_pin(SPEED_TURN_PIN, OUTPUT);
	
	config_pin(SHIFT_DATA_PIN, OUTPUT);
	config_pin(SHIFT_PIN, OUTPUT);
	config_pin(SHIFT_LATCH_PIN, OUTPUT);
	
	config_pin(1, OUTPUT);
	config_pin(2, OUTPUT);
	config_pin(3, OUTPUT);

	_shift_out();
	
	TCCR0B |= (1 << WGM02) | (1 << CS01) | (1 << CS00);
	
	sei();
}


uint8_t control_cycle()
{
	uint8_t ret = NO_ACTION;
	
	if (switch_activ_0_on && timer_1ms > switch_0_wait) {
		write_pin(SWITCH_ACTIV_PIN_0, OFF);
		switch_activ_0_on = 0;
		--timer_1ms_in_use;
	}
	
	if (switch_activ_1_on && timer_1ms > switch_1_wait) {
		write_pin(SWITCH_ACTIV_PIN_1, OFF);
		switch_activ_1_on = 0;
		--timer_1ms_in_use;
	}
	
	if (switch_activ_2_on && timer_1ms > switch_2_wait) {
		write_pin(SWITCH_ACTIV_PIN_2, OFF);
		switch_activ_2_on = 0;
		--timer_1ms_in_use;
	}

	if (speed_change_state > 0 && speed_wait > timer_1ms) {
		--speed_change_state;
		
		switch (speed_change_state) {
			case SPEED_WAIT_FOR_1_OFF:
			write_pin(SPEED_TURN_PIN, 1);
			speed_wait = timer_1ms + 20;
			break;
			
			case SPEED_WAIT_FOR_2_ON:
			write_pin(SPEED_TURN_PIN, 0);
			speed_wait = timer_1ms + 40;
			break;
			
			case SPEED_END:
			write_pin(speed_pins[next_speed], 1);
			--timer_1ms_in_use;
			break;
		}
	}

	for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
		if (timer_100ms > sensor_cooldown[i]) {
			uint8_t sensor_value = 0;
			
			for (uint8_t r = 0; r < 10; ++r) {
				if (read_pin(sensor_pins[i]) == 0)
					++sensor_value;
			}

			if (sensor_value > 5) {
				for (uint8_t j = 0; j < EVT_QUEUE_LEN; ++j) {
					if (!events[j].wait) {
						_exec_event(&(events[j]));
					}
					else {
						events[j].activated = 1;
						events[j].wait_point = events[j].wait + timer_100ms;
					}
				}
				
				sensor_cooldown[i] = timer_100ms + 10;
				
				ret = i;
				break;
			}
		}
	}
	
	for (uint8_t i = 0; i < EVT_QUEUE_LEN; ++i) {
		if (events[i].activated && timer_100ms > events[i].wait_point) {
			_exec_event(&(events[i]));
		}
	}

	return ret;
}


void _exec_event(event_t *evt)
{
	switch(evt->type) {
		case EVT_AREA:
		write_pin(area_pins[evt->own_id], evt->new_state);
		break;
		
		case EVT_SPEED:
		if (speed_change_state == SPEED_END) {
			for (uint8_t i = 0; i <= SPEED_COUNT; ++i) {
				write_pin(speed_pins[i], 0);
			}
			
			if (evt->new_state == 0) {
				speed_change_state = SPEED_WAIT_FOR_1_OFF;
			}
			else {
				speed_change_state = SPEED_WAIT_FOR_2_ON;
			}
			
			++timer_1ms_in_use;
			speed_wait = timer_1ms + 20;
		}
		break;
		
		case EVT_SWITCH:
		write_pin(switch_pins[evt->own_id], evt->new_state);
		
		uint8_t switch_activ = (evt->own_id - (evt->own_id % 2)) / 2;
		
		++timer_1ms_in_use;
		
		switch(switch_activ)
		{
			case 0:
			led_show_err(ON);
			write_pin(SWITCH_ACTIV_PIN_0, ON);
			switch_0_wait = timer_1ms + SWITCH_WAIT;
			switch_activ_0_on = 1;
			break;
			
			case 1:
			write_pin(SWITCH_ACTIV_PIN_1, ON);
			switch_1_wait = timer_1ms + SWITCH_WAIT;
			switch_activ_1_on = 1;
			break;
			
			case 2:
			default:
			write_pin(SWITCH_ACTIV_PIN_2, ON);
			switch_2_wait = timer_1ms + SWITCH_WAIT;
			switch_activ_2_on = 1;
			break;
		}
		
		break;
	}
	
	evt->activated = 0;
	evt->type = EVT_NONE;
}


uint8_t create_event(event_t* evt, uint8_t type, uint8_t own_id, uint8_t new_state, uint8_t sensor_id, uint16_t wait)
{
	uint8_t args_valid = own_id >= 0 && sensor_id >= 0 && sensor_id < SENSOR_COUNT && type > EVT_NONE && type <= EVT_SWITCH;
	
	if (args_valid) {
		switch (type)
		{
			case EVT_AREA:
			args_valid = own_id < AREA_COUNT && (new_state == 1 || new_state == 0);
			break;
			
			case EVT_SWITCH:
			args_valid = own_id < SWITCH_COUNT && (new_state == 1 || new_state == 0);
			break;
			
			case EVT_SPEED:
			args_valid = own_id < SPEED_COUNT;
			
			default:
			args_valid = 0;
		}
		
		if (args_valid) {
			evt->type = type;
			evt->own_id = own_id;
			evt->new_state = new_state;
			evt->sensor_id = sensor_id;
			evt->wait = wait / 10;
			
			return 0;
		}
	}
	
	return 1;
}


uint8_t attach_event(event_t* evt)
{
	for (uint8_t i = 0; i < 16; ++i) {
		if (events[i].type == EVT_NONE) {
			events[i].type = evt->type;
			events[i].activated = 0;
			events[i].new_state = evt->new_state;
			events[i].own_id = evt->own_id;
			events[i].sensor_id = evt->sensor_id;
			events[i].wait = evt->wait;
			
			return 0;
		}
	}
	
	return 1;
}


void config_pin(uint8_t pin, uint8_t type)
{
	if (pin < 32 && type >= 0 && type <= 2) {
		uint8_t bit_in_reg = (pin % 8);
		uint8_t reg = (pin - bit_in_reg) / 8;
		
		volatile uint8_t *port_reg, *ddr_reg;

		switch (reg) {
			case 0:
			port_reg = &PORTA;
			ddr_reg = &DDRA;
			break;

			case 1:
			port_reg = &PORTB;
			ddr_reg = &DDRB;
			break;

			case 2:
			port_reg = &PORTC;
			ddr_reg = &DDRC;
			break;

			case 3:
			default:
			port_reg = &PORTD;
			ddr_reg = &DDRD;
		}
		
		if (type == OUTPUT) {
			*ddr_reg |= (1 << bit_in_reg);
		}
		else {
			*ddr_reg &= ~(1 << bit_in_reg);

			if (type == INPUT) {
				*port_reg &= ~(1 << bit_in_reg);
			}
			else {
				*port_reg |= (1 << bit_in_reg);
			}
		}
	}
}


void write_pin(uint8_t pin, uint8_t new_state)
{
	uint8_t bit_in_reg = (pin % 8);
	uint8_t reg = (pin - bit_in_reg) / 8;

	if (pin < 32 && (new_state == 0 || new_state == 1)) {
		volatile uint8_t *port_reg;
		switch (reg) {
			case 0:
			port_reg = &PORTA;
			break;
			
			case 1:
			port_reg = &PORTB;
			break;
			
			case 2:
			port_reg = &PORTC;
			break;
			case 3:
			default:
			port_reg = &PORTD;
		}
		
		if (new_state) {
			*port_reg |= (1 << bit_in_reg);
		}
		else {
			*port_reg &= ~(1 << bit_in_reg);
		}
	}
	else if (reg < (4 + SHIFT_REG_COUNT)) {
		if (new_state) {
			shifts[reg - 4] |= (1 << (7 - bit_in_reg));
		}
		else {
			shifts[reg - 4] &= ~(1 << (7 - bit_in_reg));
		}
		
		_shift_out();
	}
}


uint8_t read_pin(uint8_t pin)
{
	uint8_t bit_in_reg = (pin % 8);
	uint8_t reg = (pin - bit_in_reg) / 8;

	if (pin < 32) {
		switch (reg) {
			case 0:
			return (((1 << bit_in_reg) & PINA) >> bit_in_reg);
			break;
			
			case 1:
			return (((1 << bit_in_reg) & PINB) >> bit_in_reg);
			break;
			
			case 2:
			return (((1 << bit_in_reg) & PINC) >> bit_in_reg);
			break;
			
			case 3:
			return (((1 << bit_in_reg) & PIND) >> bit_in_reg);
			break;
			
			default:
			return 0;
		}
	}
	
	return 1;
}


void _shift_out()
{
	cli();
	for (int8_t i = SHIFT_REG_COUNT - 1; i >= 0; --i) {
		for (uint8_t j = 0; j < 8; ++j) {
			write_pin(SHIFT_DATA_PIN, (shifts[i] & (1 << j)) >> j);

			write_pin(SHIFT_PIN, 1);
			write_pin(SHIFT_PIN, 0);
		}
	}

	write_pin(SHIFT_LATCH_PIN, 1);
	write_pin(SHIFT_LATCH_PIN, 0);
	sei();
}


ISR (TIMER0_COMPA_vect)
{
	uint16_t tmp_timer_1ms = timer_1ms;
	
	if ((tmp_timer_1ms % 100) == 0) {
		++timer_100ms;
	}
	
	++tmp_timer_1ms;
	
	if ((tmp_timer_1ms % 100) == 99 && !timer_1ms_in_use) {
		tmp_timer_1ms = 0;
	}
	
	timer_1ms = tmp_timer_1ms;
}


void led_show_err(uint8_t err_code)
{
	if (err_code) {
		write_pin(1, ON);
	}
	else {
		write_pin(1, OFF);
	}
}


void led_show_success(uint8_t enable)
{
	if (enable) {
		write_pin(2, ON);
	}
	else {
		write_pin(2, OFF);
	}
}


void led_show_waiting(uint8_t enable)
{
	if (enable) {
		write_pin(3, ON);
	}
	else {
		write_pin(3, OFF);
	}
}