#ifndef MAERKLIN_COM_H
#define MAERKLIN_COM_H

#define BAUD_RATE 38400UL
#define BAUD_PRESCALE 25U
#define L_TX_BUF 512
#define L_RX_BUF 512

#include <avr/io.h>
#include <avr/interrupt.h>

typedef struct msg_t {
	uint8_t len;
	uint8_t type;
	uint8_t goal;
	uint8_t id;
	uint8_t new_state;
	uint8_t* msg;
} msg_t;

static const uint8_t welcome_msg[] = "Welcome to the model railroad controller!\n There is currently nothing implemented which can be called from your PC\n";

void init_maerklin_com();


#endif