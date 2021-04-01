#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>


#include <main.h>
#include <motors.h>
#include <pi_regulator.h>
#include <process_image.h>

#define MAX_ERROR_SUM 15
#define MIDDLE 320

static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    int16_t speed = 0;
    int16_t speed_left = 0;
    int16_t speed_right = 0;

    float error ; //pos si  trop loin neg si trop proche
    float Kp = 200;
	float Ki = 10;
	float error_sum = 0;

    while(1){
        time = chVTGetSystemTime();

        error = get_distance_cm()-10;
        error_sum = error_sum + error;
        if (error_sum > MAX_ERROR_SUM){
        	error_sum = MAX_ERROR_SUM;
        }
        else if(error_sum < -MAX_ERROR_SUM){
                	error_sum = -MAX_ERROR_SUM;
                }

        speed =  Kp*error +Ki*error_sum ;
        if (get_distance_cm() == 17){
        	speed= 0;
        }
        
        speed_right = speed + (MIDDLE - get_position_px());
        speed_left = speed - (MIDDLE - get_position_px());

        //applies the speed from the PI regulator
		 right_motor_set_speed(speed_right);
		 left_motor_set_speed(speed_left);

        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(100));
    }
}

void pi_regulator_start(void){
	chThdCreateStatic(waPiRegulator, sizeof(waPiRegulator), NORMALPRIO, PiRegulator, NULL);
}
