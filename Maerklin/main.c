#include "maerklin_std.h"
#include "maerklin_com.h"

int main(void)
{
	init_maerklin_std();
	
	event_t switchEvt;
	
	create_event(&switchEvt, EVT_SWITCH, 0, RIGHT, 0, 0);
	attach_event(&switchEvt);
	
	while (1) {
		uint8_t sensor = control_cycle();
	}
	
	return 1;
}
