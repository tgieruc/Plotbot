#include "sensors/VL53L0X/VL53L0X.h"
#include "sensors/proximity.h"
#include <main.h>
#include <chprintf.h>

#include <motors.h>
#include <chthreads.h>
#include <smartmove.h>
#include <audio_processing.h>
#include <process_image.h>
#include <audio/play_sound_file.h>
#include <leds_animations.h>

#define ANGLE2STEP 	3.6f
#define NORTH 		0
#define EAST 		90
#define SOUTH 		180
#define WEST 		270

#define DIST_SENSOR_TUBE 	40
#define BLIND_TURN_SPEED 	300
#define CENTERING_SPEED 	120
#define FORWARD_SPEED		500
#define ERROR_MARGIN		10
#define SENSORS_FRONT_RIGHT 	0
#define SENSORS_FRONT_LEFT 	 	7
#define CLOSE_DIST				340
#define KP_TURN					0.3
#define KP_FORWARD 				1
#define KI 						0.2	//must not be zero
#define MAX_SUM_ERROR 			50

#define MIDDLE_DCIM 320

static uint8_t absPosition[][2]  = {{0,0}, {0,1}, {0,2},
									{1,0}, {1,1}, {1,2},
									{2,0}, {2,1}, {2,2}};

typedef struct smartinfo_t{
	int16_t actual_direction;
	int16_t angle;
	uint16_t dist_to_tube;
}smartinfo_t;


void get_smart_info(uint8_t actualPos, uint8_t nextPos, smartinfo_t *smartinfo);
void centering(void);
void smart_move(smartinfo_t *smartinfo);
void blind_turn(smartinfo_t *smartinfo);
void set_dist_to_tube(smartinfo_t *smartinfo, uint8_t actualPosX, uint8_t actualPosY);
void move_forward(smartinfo_t *smartinfo);
bool must_stop(smartinfo_t *smartinfo);
uint16_t get_next_direction(int8_t deltaPosX, int8_t deltaPosY);


static THD_WORKING_AREA(waThdSmartMove, 512);
static THD_FUNCTION(ThdSmartMove, arg) {

	chRegSetThreadName(__FUNCTION__);
	(void)arg;
	int8_t sequ[MAX_MOVES];
	uint8_t sequ_size = 0;


	wait_sequ_aquired();
	get_sequ(&sequ_size, sequ);
	chThdSleepMilliseconds(500);
	setSoundFileVolume(50);
//	chSysLock();
	playSoundFile("letsgo.wav",SF_SIMPLE_PLAY);
	waitSoundFileHasFinished();
//	chSysUnlock();
	set_led_state(MOVING);


	chprintf((BaseSequentialStream *) &SD3, "\nSequ: [ ");
	for (uint i = 0; i < sequ_size; ++i){
		chprintf((BaseSequentialStream *) &SD3, "%d ", sequ[i]);
	}
	chprintf((BaseSequentialStream *) &SD3, "]\n");
	smartinfo_t smartinfo;
	smartinfo.actual_direction = NORTH;


	for (uint8_t i = 0; i < sequ_size-1; ++i){
		get_smart_info(sequ[i],sequ[i+1],&smartinfo);
		chprintf((BaseSequentialStream *)&SD3,
			      "smartinfo : angle %d, dist %d\n",
				  smartinfo.angle, smartinfo.dist_to_tube);

		smart_move(&smartinfo);
	}
	set_led_state(DONE);
}

/*
 * Handle all the movement functions
 */
void smart_move(smartinfo_t *smartinfo){
	if (smartinfo->angle != 0) {
		blind_turn(smartinfo);
	}
  	centering();
	move_forward(smartinfo);
}

/*
 * Check if the robot must continue to go forward
 */
bool must_stop(smartinfo_t *smartinfo){
    messagebus_topic_t *prox_topic = messagebus_find_topic_blocking(&bus, "/proximity");

    proximity_msg_t prox_values;

    if (smartinfo->dist_to_tube > 200){
    	return (VL53L0X_get_dist_mm() < smartinfo->dist_to_tube-100);
    }
    if (VL53L0X_get_dist_mm() > 60) {
    	return false;
    }

	messagebus_topic_wait(prox_topic, &prox_values, sizeof(prox_values));

    return ((prox_values.delta[SENSORS_FRONT_RIGHT] + prox_values.delta[SENSORS_FRONT_LEFT])/2 > CLOSE_DIST);
}


/*
 * Return the direction the epuck will have after its turn
 */
uint16_t get_next_direction(int8_t deltaPosX, int8_t deltaPosY){
	if (deltaPosX ==  1) return SOUTH;
	if (deltaPosX == -1) return NORTH;
	if (deltaPosY ==  1) return EAST;
	return WEST;
}

/*
 * Generate all the informations for the move
 * - the angle it has to do
 * - the direction it will face
 * - the distance between the opposing tube and the TOF
 */
void get_smart_info(uint8_t actualPos, uint8_t nextPos, smartinfo_t *smartinfo){
	int16_t nextDirection;
	int8_t deltaPosX = absPosition[nextPos-1][0] - absPosition[actualPos-1][0];
	int8_t deltaPosY = absPosition[nextPos-1][1] - absPosition[actualPos-1][1];
	nextDirection = get_next_direction(deltaPosX, deltaPosY);

	smartinfo->angle = nextDirection - smartinfo->actual_direction;
	smartinfo->actual_direction = nextDirection;

	if (smartinfo->angle ==  270) smartinfo->angle = -90;
	if (smartinfo->angle == -270) smartinfo->angle =  90;

	set_dist_to_tube(smartinfo, absPosition[actualPos-1][0], absPosition[actualPos-1][1]);
}

/*
 * Compute the distance between the opposing tube and the TOF according to its position and direction
 */
void set_dist_to_tube(smartinfo_t *smartinfo, uint8_t actualPosX, uint8_t actualPosY){
	if (smartinfo->actual_direction == EAST || smartinfo->actual_direction == WEST){
		if (actualPosY == 0 || actualPosY == 2){
			smartinfo->dist_to_tube = 2*100+DIST_SENSOR_TUBE;
		} else {
			smartinfo->dist_to_tube = 100+DIST_SENSOR_TUBE;
		}
	} else {
		if (actualPosX == 0 || actualPosX == 2){
			smartinfo->dist_to_tube = 2*100 + DIST_SENSOR_TUBE;
		} else {
			smartinfo->dist_to_tube = 100 + DIST_SENSOR_TUBE;
		}
	}
}

/*
 * Calculate the number of steps to turn and turn
 */
void blind_turn(smartinfo_t *smartinfo){
	int8_t rotary_direction;
	uint16_t step2do;
	left_motor_set_pos(0);
	right_motor_set_pos(0);

	rotary_direction = abs(smartinfo->angle) / smartinfo->angle;
	step2do = ANGLE2STEP * (abs(smartinfo->angle));

	while (abs(left_motor_get_pos()) < step2do){
		left_motor_set_speed(BLIND_TURN_SPEED * rotary_direction);
		right_motor_set_speed(-BLIND_TURN_SPEED * rotary_direction);
	}
	left_motor_set_speed(0);
	right_motor_set_speed(0);
}

/*
 * Turn according to the TOF sensor
 */
void centering(void){
	float error = MIDDLE_DCIM - get_position_px();
	float speed = 0;
	float sum_error = 0;
	chprintf((BaseSequentialStream *) &SD3, "smartturn \n");


	while (abs (error) > ERROR_MARGIN){
		wait_position_acquired();
		sum_error += error;

		if(sum_error > MAX_SUM_ERROR){
			sum_error = MAX_SUM_ERROR;
		}else if(sum_error < -MAX_SUM_ERROR){
			sum_error = -MAX_SUM_ERROR;
		}

		speed = error * KP_TURN + sum_error*KI;

		if (speed > CENTERING_SPEED) speed = CENTERING_SPEED;
		if (speed < -CENTERING_SPEED) speed = -CENTERING_SPEED;

		left_motor_set_speed(-speed);
		right_motor_set_speed(speed);
		error = MIDDLE_DCIM - get_position_px();
	}

	left_motor_set_speed(0);
	right_motor_set_speed(0);
}

/*
 * Goes forward and uses PI to stay centered on the line
 */
void move_forward(smartinfo_t *smartinfo){
	while (!must_stop(smartinfo)) {
		left_motor_set_speed(FORWARD_SPEED);
		right_motor_set_speed(FORWARD_SPEED);
	}

	left_motor_set_speed(0);
	right_motor_set_speed(0);
}


void smartmove_start(void){
	chThdCreateStatic(waThdSmartMove, sizeof(waThdSmartMove), NORMALPRIO, ThdSmartMove, NULL);
}

