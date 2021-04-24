#include "sensors/VL53L0X/VL53L0X.h"
#include <main.h>
#include <chprintf.h>

#include <motors.h>
#include <chthreads.h>
#include <smartmove.h>


#define ANGLE2STEP 	3.7591706966f
#define ANGLEMARGIN 20
#define NORTH 		0
#define EAST 		90
#define SOUTH 		180
#define WEST 		270

#define TOF_NOISE_MARGIN 	3
#define DIST2TUBE_MARGIN 	30
#define DISTSENSORTUBE 		40
#define TOF_REFRESH_RATE 	30
#define BLIND_TURN_SPEED 	300
#define SMART_TURN_SPEED 	200

static uint8_t absPosition[][2]  = {{0,0}, {0,1}, {0,2}, {1,0}, {1,1}, {1,2}, {2,0}, {2,1}, {2,2}};

typedef struct smart_info_t{
	int16_t actualDirection;
	int16_t angle;
	uint16_t distToTube;
}smart_info_t;


void get_smart_info(uint8_t actualPos, uint8_t nextPos, smart_info_t *smartinfo);
void smart_turn(smart_info_t *smartinfo);
void smart_move(smart_info_t *smartinfo);
uint16_t get_next_direction(int8_t deltaPosX, int8_t deltaPosY);
void blind_turn(smart_info_t *smartinfo);
void set_dist_to_tube(smart_info_t *smartinfo, uint8_t actualPosX, uint8_t actualPosY);




static THD_WORKING_AREA(waThdSmartMove, 1024);
static THD_FUNCTION(ThdSmartMove, arg) {

	chRegSetThreadName(__FUNCTION__);
	(void)arg;

	smart_info_t smartinfo;
	smartinfo.actualDirection = EAST;

	uint8_t actualPos, nextPos;
	actualPos = 2;
	nextPos = 3;

	get_smart_info(actualPos,nextPos,&smartinfo);
	smart_move(&smartinfo);

    while(1){
//    	chprintf((BaseSequentialStream *) &SD3, "dist %d \n",VL53L0X_get_dist_mm());
    }
}

void smart_move(smart_info_t *smartinfo){
	blind_turn(smartinfo);
  	smart_turn(smartinfo);
//	move()
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
void get_smart_info(uint8_t actualPos, uint8_t nextPos, smart_info_t *smartinfo){
	int16_t nextDirection;
	int8_t deltaPosX = absPosition[nextPos-1][0] - absPosition[actualPos-1][0];
	int8_t deltaPosY = absPosition[nextPos-1][1] - absPosition[actualPos-1][1];
	nextDirection = get_next_direction(deltaPosX, deltaPosY);

	smartinfo->angle = nextDirection - smartinfo->actualDirection;
	smartinfo->actualDirection = nextDirection;

	set_dist_to_tube(smartinfo, absPosition[actualPos-1][0], absPosition[actualPos-1][1]);
}

/*
 * Compute the distance between the opposing tube and the TOF according to its position and direction
 */
void set_dist_to_tube(smart_info_t *smartinfo, uint8_t actualPosX, uint8_t actualPosY){
	if (smartinfo->actualDirection == EAST || smartinfo->actualDirection == WEST){
		if (actualPosX == 0 || actualPosX == 2){
			smartinfo->distToTube = 2*100+DISTSENSORTUBE;
		} else {
			smartinfo->distToTube = 100+DISTSENSORTUBE;
		}
	} else {
		if (actualPosY == 0 || actualPosY == 2){
			smartinfo->distToTube = 2*100 + DISTSENSORTUBE;
		} else {
			smartinfo->distToTube = 100 + DISTSENSORTUBE;
		}
	}
}

/*
 * Calculate the number of steps to turn and turn
 */
void blind_turn(smart_info_t *smartinfo){
	int8_t rotary_direction;
	uint16_t step2do;
	left_motor_set_pos(0);
	right_motor_set_pos(0);

	if (smartinfo->angle == 0) {
		rotary_direction = 1;
		step2do = ANGLE2STEP * 45;
	} else {
		rotary_direction = abs(smartinfo->angle) / smartinfo->angle;
		step2do = ANGLE2STEP * (abs(smartinfo->angle  - ANGLEMARGIN));
	}

	while (abs(left_motor_get_pos()) < step2do){
		left_motor_set_speed(BLIND_TURN_SPEED * rotary_direction);
		right_motor_set_speed(-BLIND_TURN_SPEED * rotary_direction);
	}

	if (smartinfo->angle == 0) rotary_direction = -rotary_direction;
	left_motor_set_speed(SMART_TURN_SPEED * rotary_direction);
	right_motor_set_speed(-SMART_TURN_SPEED * rotary_direction);
}

/*
 * Turn according to the TOF sensor
 */
void smart_turn(smart_info_t *smartinfo){
	uint16_t min_dist = 10000;
	uint16_t dist = VL53L0X_get_dist_mm();
	systime_t time;
	while (dist > smartinfo->distToTube + DIST2TUBE_MARGIN ||
		   dist < smartinfo->distToTube - DIST2TUBE_MARGIN){
		dist = VL53L0X_get_dist_mm();
		chprintf((BaseSequentialStream *) &SD3, "expected : %d, got %d\n",smartinfo->distToTube,dist);
	}
	while (dist < min_dist + TOF_NOISE_MARGIN){
		chprintf((BaseSequentialStream *) &SD3, "boucle mindidst\n");
		min_dist = dist;
		time = chVTGetSystemTime();
		chThdSleepUntilWindowed(time, time + MS2ST(TOF_REFRESH_RATE));
		dist = VL53L0X_get_dist_mm();
	}
	left_motor_set_speed(0);
	right_motor_set_speed(0);
}


void smartmove_start(void){
	chThdCreateStatic(waThdSmartMove, sizeof(waThdSmartMove), NORMALPRIO, ThdSmartMove, NULL);
}

