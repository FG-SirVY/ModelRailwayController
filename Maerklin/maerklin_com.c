#include "maerklin_com.h"

volatile uint8_t msg_index_to_send;
volatile uint8_t msg_length;
volatile uint8_t send_buffer[L_TX_BUF] = {0};
volatile uint8_t rcv_buffer[L_RX_BUF] = {0};
volatile uint8_t msg_send_complete;


void init_maerklin_com()
{
	UCSR0B |= (1 << UDRIE0) | (1 << RXCIE0) | (1 << TXEN0) | (1 << RXEN0);
	UCSR0C |= (1 << UPM10) | (1 << USBS0) | (1 << UCSZ01) | (1 << UCSZ00);
	UBRR0 = BAUD_PRESCALE;
	
	msg_send_complete = 0;
	msg_index_to_send = 0;
	msg_length = 0;
}

/*uint8_t get_msg_params(uint8_t* buf, uint8_t buf_len, msg_t* msg)
{
	if (buf_len < 64 || buf == NULL || msg == NULL) {
		return 1;
	}
	
	msg->len = buf[0] << 8;
	msg->len |= buf[1];
	msg->type = buf[2];
	msg->goal = (buf[3] > 128);
	msg->new_state = (buf[5] > 128);
	
	if (!msg->type) {
		msg->msg = "Test success";
	}
	
	return 0;
}


uint8_t create_msg(uint8_t* buf, uint8_t buf_len, msg_t* msg)
{
	
}*/


ISR (USART0_UDRE_vect)
{
	if (!msg_send_complete) {
		uint8_t tmp_msg_index = msg_index_to_send;
		
		if (tmp_msg_index < L_TX_BUF) {
			UDR0 = send_buffer[tmp_msg_index];
			++tmp_msg_index;
			
			if (tmp_msg_index == msg_length) {
				tmp_msg_index = 0;
				msg_send_complete = 1;
			}
			
			msg_index_to_send = tmp_msg_index;
		}
	}
}


ISR (USART0_RX_vect)
{
	if (msg_send_complete) {
		msg_length = sizeof(welcome_msg) / sizeof(uint8_t);
		
		for (uint8_t i = 0; i < msg_length; ++i) {
			send_buffer[i] = welcome_msg[i];
		}
		
		msg_index_to_send = 0;
		UDR0 = send_buffer[msg_index_to_send];
		++msg_index_to_send;
	}
}
