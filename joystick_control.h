// module to handle zen-cape joystick inputs
#ifndef JOYSTICK_CONTROL_H
#define JOYSTICK_CONTROL_H

enum direction{UP, RIGHT, DOWN, LEFT, PRESS_IN, NO_DIRECTION};

void JoystickControl_init(void);
void JoystickControl_cleanup(void);

#endif
