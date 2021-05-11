/*
 * leds_animations.h
 *
 *  Created on: 2 mai 2021
 *      Author: guill
 */

#ifndef LEDS_ANIMATIONS_H_
#define LEDS_ANIMATIONS_H_

typedef enum{
	IDLE = 0, //waiting for the start sequence
	LISTENING,//listening to the sequence¨
	MOVING,
	DONE,
	ERROR_MODE,
}leds_animation_t;


void set_led_state(leds_animation_t new_animation);

void leds_animations_start(void);

#endif /* LEDS_ANIMATIONS_H_ */
