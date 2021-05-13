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
#include <main.h>

static leds_animation_t animation = IDLE;

//*****FORWARD DECLARATION*****
void set_led_state (leds_animation_t new_animation);
void leds_animations_start(void);
//*****************************

static THD_WORKING_AREA(waThdLedsAnimations, 256);
static THD_FUNCTION(ThdLedsAnimations, arg) {

	chRegSetThreadName(__FUNCTION__);
	(void)arg;

	uint8_t active_led = 0;

	clear_leds();

	while(1){
		clear_leds();
		switch(animation){
		case IDLE: //rotating leds
			#ifdef DEBUG
			chprintf((BaseSequentialStream *) &SD3, "led mode : idle \n");
			#endif
			set_led(((active_led++)%4), 1);
			chThdSleepMilliseconds(500);
			break;
		case LISTENING://pulsing body led
			#ifdef DEBUG
			chprintf((BaseSequentialStream *) &SD3, "led mode : listening \n");
			#endif
			set_body_led(active_led++%2);
			chThdSleepMilliseconds(200);
			break;
		case MOVING://solid body led
			#ifdef DEBUG
			chprintf((BaseSequentialStream *) &SD3, "led mode : moving \n");
			#endif
			set_body_led(1);
			break;
		case DONE://body led off
			#ifdef DEBUG
			chprintf((BaseSequentialStream *) &SD3, "led mode : done \n");
			#endif
			set_body_led(0);
			break;
		case ERROR_MODE://everything blinks
			#ifdef DEBUG
			chprintf((BaseSequentialStream *) &SD3, "led mode : error \n");
			#endif
			set_body_led(active_led++%2);
			set_led(4,active_led%2);
			chThdSleepMilliseconds(200);;
			break;
		}
	}
}

void set_led_state (leds_animation_t new_animation){
	animation = new_animation;
}

void leds_animations_start(void){
	chThdCreateStatic(waThdLedsAnimations, sizeof(waThdLedsAnimations), NORMALPRIO, ThdLedsAnimations, NULL);
}

