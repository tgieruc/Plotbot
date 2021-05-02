/*
 * leds_animations.c
 *
 *  Created on: 2 mai 2021
 *      Author: guill
 *
 */
#include <leds.h>
#include <leds_animations.h>
#include <chprintf.h>

static leds_animation_t animation = IDLE;

void set_led_state (leds_animation_t new_animation){
	animation = new_animation;

}

static THD_WORKING_AREA(waThdLedsAnimations, 256);
static THD_FUNCTION(ThdLedsAnimations, arg) {

	chRegSetThreadName(__FUNCTION__);
	(void)arg;

	uint8_t active_led = 0;
	uint8_t pulse_state = 0;

	clear_leds();

//	set_led(4,1); test

	while(1){

		switch(animation){
		case IDLE: //solid blue
			chprintf((BaseSequentialStream *) &SD3, "led mode : idle");
			set_rgb_led(0, 0, 0, 10);
			set_rgb_led(1, 0, 0, 10);
			set_rgb_led(2, 0, 0, 10);
			set_rgb_led(3, 0, 0, 10);
			break;
		case LISTENING://solid green
			chprintf((BaseSequentialStream *) &SD3, "led mode : listening");
			set_rgb_led(0, 0, 10, 0);
			set_rgb_led(1, 0, 10, 0);
			set_rgb_led(2, 0, 10, 0);
			set_rgb_led(3, 0, 10, 0);
			break;
		case MOVING://rotating green
			chprintf((BaseSequentialStream *) &SD3, "led mode : moving");
			clear_leds();
			set_rgb_led(((active_led++)%4), 0, 10, 0);
			chThdSleepMilliseconds(200);
			break;
		case DONE://pulsing blue
			chprintf((BaseSequentialStream *) &SD3, "led mode : done");
			set_rgb_led(0, 0, 0, pulse_state);
			set_rgb_led(1, 0, 0, pulse_state);
			set_rgb_led(2, 0, 0, pulse_state);
			set_rgb_led(3, 0, 0, pulse_state);
			if (active_led%20 < 10){
				pulse_state = active_led%10;
			}
			else{
				pulse_state = 10-active_led%10;
			}
			break;
		}
		chThdYield();
	}
}

void leds_animations_start(void){
	chThdCreateStatic(waThdLedsAnimations, sizeof(waThdLedsAnimations), NORMALPRIO, ThdLedsAnimations, NULL);
}

