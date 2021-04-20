#include "sensors/VL53L0X/VL53L0X.h"
#include <main.h>
#include <chprintf.h>

#include <motors.h>
#include <chthreads.h>
#include <tof.h>


#define ANGLE2STEP 3.7591706966f
#define ANGLEMARGIN 15

enum Direction {LEFT_ST = 1, RIGHT_ST = -1};



static THD_WORKING_AREA(waThdTOF, 1024);
static THD_FUNCTION(ThdTOF, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    smart_turn(LEFT_ST,90,90,140);


    while(1){
		chprintf((BaseSequentialStream *) &SD3, "dist %d \n",VL53L0X_get_dist_mm());

    }
}




void smart_turn(int8_t direction, uint16_t angle, uint16_t expected_min_dist, uint16_t expected_max_dist){
	//blind turn angle-15
	left_motor_set_pos(0);
	right_motor_set_pos(0);
	uint16_t step2do = ANGLE2STEP * (angle - ANGLEMARGIN);
	while (abs(left_motor_get_pos()) < step2do){
		left_motor_set_speed(-300 * direction);
		right_motor_set_speed(300 * direction);
	}

	left_motor_set_speed(-200 * direction);
	right_motor_set_speed(200 * direction);
	//smart turn
	uint16_t min_dist = 10000;
	uint16_t dist = VL53L0X_get_dist_mm();
	systime_t time;
	while (dist > expected_max_dist || dist < expected_min_dist){
		dist = VL53L0X_get_dist_mm();
		chprintf((BaseSequentialStream *) &SD3, "boucle expected\n");

	}
	while (!(dist > min_dist)){
		chprintf((BaseSequentialStream *) &SD3, "boucle mindidst\n");
		min_dist = dist;
		time = chVTGetSystemTime();
		chThdSleepUntilWindowed(time, time + MS2ST(35));
		dist = VL53L0X_get_dist_mm();
	}
	left_motor_set_speed(0);
	right_motor_set_speed(0);

}


void tof_start(void){
	chThdCreateStatic(waThdTOF, sizeof(waThdTOF), NORMALPRIO, ThdTOF, NULL);
}

