#ifndef _UI_H
#define _UI_H

#include <stdint.h>
#include <stdbool.h>
#include "label.h"

extern label_t lblLeftJoy, lblRightJoy, lblFlightMode;
extern label_t ui_buttonLabel[6];

void ui_load(void);
void ui_update(uint8_t isArmed, const char* flightmode);
void ui_button_highlight(uint8_t button_id, bool highlighted);

#endif