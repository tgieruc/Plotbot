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

	clear_leds();

//	set_led(4,1); test

	while(1){
		clear_leds();
		switch(animation){
		case IDLE: //rotating leds
			chprintf((BaseSequentialStream *) &SD3, "led mode : idle");
			set_led(((active_led++)%4), 1);
			chThdSleepMilliseconds(500);
			break;
		case LISTENING://pulsing body led
			chprintf((BaseSequentialStream *) &SD3, "led mode : listening");
			set_body_led(active_led++%2);
			chThdSleepMilliseconds(200);
			break;
		case MOVING://solid body led
			chprintf((BaseSequentialStream *) &SD3, "led mode : moving");
			set_body_led(1);
			break;
		case DONE://body led off
			chprintf((BaseSequentialStream *) &SD3, "led mode : done");
			set_body_led(0);
			break;
		}
//		chThdYield();
	}
}

void leds_animations_start(void){
	chThdCreateStatic(waThdLedsAnimations, sizeof(waThdLedsAnimations), NORMALPRIO, ThdLedsAnimations, NULL);
}

