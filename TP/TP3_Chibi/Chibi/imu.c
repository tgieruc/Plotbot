#include <ch.h>
#include <hal.h>
#include <math.h>
#include <messagebus.h>
#include <imu.h>

#define STANDARD_GRAVITY    9.80665f
#define DEG2RAD(deg) (deg / 180 * M_PI)

extern messagebus_t bus;

static imu_msg_t imu_values;

static thread_t *imuThd;
static bool imu_configured = false;

void floatArrayAdd(float *arraydestination, float *arrayaddition, uint16_t size_array );/***************************INTERNAL FUNCTIONS************************************/
void intArrayAdd(int16_t *arraydestination, int16_t *arrayaddition, uint16_t size_array, uint16_t nb_samples );
void floatArrayDivise(float *arraydestination, float *arraydivise, float divisor, uint16_t size_array );
void intArrayReplace(int16_t *arraydestination, int16_t *arrayreplacer,uint16_t size_array );
 /**
 * @brief   Computes the measurements of the imu into readable measurements
 * 			RAW accelerometer to m/s^2 acceleration
 * 			RAW gyroscope to rad/s speed
 */


void imu_compute_units(void){
	for (int i = 0 ; i < NB_AXIS; ++i){
		imu_values.acceleration[i] = (imu_values.acc_raw[i] - imu_values.acc_offset[i]) * 2 / 16384 * STANDARD_GRAVITY;
		imu_values.gyro_rate[i]    = DEG2RAD((imu_values.gyro_raw[i] - imu_values.gyro_offset[i]) * 250/16384);
	}
}



 /**
 * @brief   Thread which updates the measurements and publishes them
 */
static THD_FUNCTION(imu_reader_thd, arg) {
     (void) arg;
     chRegSetThreadName(__FUNCTION__);

     // Declares the topic on the bus.
     messagebus_topic_t imu_topic;
     MUTEX_DECL(imu_topic_lock);
     CONDVAR_DECL(imu_topic_condvar);
     messagebus_topic_init(&imu_topic, &imu_topic_lock, &imu_topic_condvar, &imu_values, sizeof(imu_values));
     messagebus_advertise_topic(&bus, &imu_topic, "/imu");

     systime_t time;

     while (chThdShouldTerminateX() == false) {
    	 time = chVTGetSystemTime();

    	if(imu_configured == true){
	 		/* Reads the incoming measurement. */
			mpu9250_read(imu_values.gyro_raw, imu_values.acc_raw, &imu_values.status);
			/* computes the raw values into readable values*/
			imu_compute_units();
     	}

     	/* Publishes it on the bus. */
		messagebus_topic_publish(&imu_topic, &imu_values, sizeof(imu_values));

        chThdSleepUntilWindowed(time, time + MS2ST(4)); //reduced the sample rate to 250Hz

     }
}

/*************************END INTERNAL FUNCTIONS**********************************/


/****************************PUBLIC FUNCTIONS*************************************/

void imu_start(void)
{
	int8_t status = MSG_OK;

    status = mpu9250_setup(MPU9250_ACC_FULL_RANGE_2G
		                  | MPU9250_GYRO_FULL_RANGE_250DPS
		                  | MPU9250_SAMPLE_RATE_DIV(100));
		                  //| MPU60X0_LOW_PASS_FILTER_6)

    //not tested yet because the auxilliary I2C of the MPU-9250 is condamned due
    //to PCB correction on the e-puck2-F4, so the magnetometer cannot be read...
    // if(status == MSG_OK){
    // 	status = mpu9250_magnetometer_setup();
    // }

    if(status == MSG_OK){
    	imu_configured = true;
    }

    static THD_WORKING_AREA(imu_reader_thd_wa, 1024);
    imuThd = chThdCreateStatic(imu_reader_thd_wa, sizeof(imu_reader_thd_wa), NORMALPRIO, imu_reader_thd, NULL);
}

void imu_stop(void) {
    chThdTerminate(imuThd);
    chThdWait(imuThd);
    imuThd = NULL;
}

void imu_compute_offset(messagebus_topic_t * imu_topic, uint16_t nb_samples){


    /*
    *   TASK 9 : TO COMPLETE
    */

	imu_msg_t imu_values_temp;
    float acceleration[NB_AXIS]={0,0,0}; // m/s^2
    float gyro_rate[NB_AXIS]={0,0,0}; // rad/s
    float temperature=0;
    float magnetometer[NB_AXIS]={0,0,0}; //uT
    int16_t acc_raw[NB_AXIS]={0,0,0}; //raw values
    int16_t gyro_raw[NB_AXIS]={0,0,0}; //raw values
    int16_t acc_offset[NB_AXIS]={0,0,0}; //raw offsets
    int16_t gyro_offset[NB_AXIS]={0,0,0}; //raw offsets
    int16_t acc_filtered[NB_AXIS]={0,0,0};
    int16_t gyro_filtered[NB_AXIS]={0,0,0};
    uint8_t status=0;

	for (int i = 0; i < nb_samples; ++i){
        messagebus_topic_wait(imu_topic, &imu_values_temp, sizeof(imu_values));

//        floatArrayAdd(acceleration,imu_values_temp.acceleration,NB_AXIS);
//        floatArrayAdd(gyro_rate,imu_values_temp.gyro_rate,NB_AXIS);
//        temperature += imu_values_temp.temperature;
//        floatArrayAdd(magnetometer,imu_values_temp.magnetometer,NB_AXIS);
        intArrayAdd(acc_raw,imu_values_temp.acc_raw,NB_AXIS,nb_samples);
        intArrayAdd(gyro_raw,imu_values_temp.gyro_raw,NB_AXIS,nb_samples);
//        intArrayAdd(acc_offset,imu_values_temp.acc_offset,NB_AXIS);
//        intArrayAdd(gyro_offset,imu_values_temp.gyro_offset,NB_AXIS);
//        intArrayAdd(acc_filtered,imu_values_temp.acc_filtered,NB_AXIS);
//        intArrayAdd(gyro_filtered,imu_values_temp.gyro_filtered,NB_AXIS);
//        status += imu_values_temp.status;
	}



//	floatArrayDivise(imu_values.acceleration,acceleration,nb_samples,NB_AXIS);
//	floatArrayDivise(imu_values.gyro_rate,gyro_rate,nb_samples,NB_AXIS);
//	imu_values.temperature=temperature/nb_samples;
//	floatArrayDivise(imu_values.magnetometer,magnetometer,nb_samples,NB_AXIS);
//	intArrayDivise(imu_values.acc_raw,acc_raw,nb_samples,NB_AXIS);
//	intArrayDivise(imu_values.gyro_raw,gyro_raw,nb_samples,NB_AXIS);
	intArrayReplace(imu_values.acc_offset,acc_raw,NB_AXIS);
	intArrayReplace(imu_values.gyro_offset,gyro_raw,NB_AXIS);
//	intArrayDivise(imu_values.acc_filtered,acc_filtered,nb_samples,NB_AXIS);
//	intArrayDivise(imu_values.gyro_filtered,gyro_filtered,nb_samples,NB_AXIS);
//	imu_values.status=status/nb_samples;

}

void floatArrayAdd(float *arraydestination, float *arrayaddition, uint16_t size_array ){
	for (int i = 0; i < size_array; ++i){
		arraydestination[i] += arrayaddition[i];
	}
}

void intArrayAdd(int16_t *arraydestination, int16_t *arrayaddition, uint16_t size_array, uint16_t nb_samples ){
	for (int i = 0; i < size_array; ++i){
		arraydestination[i] += arrayaddition[i]/nb_samples;
	}
}

void floatArrayDivise(float *arraydestination, float *arraydivise, float divisor, uint16_t size_array ){
	for (int i = 0; i < size_array; ++i){
		arraydestination[i] = arraydivise[i] / divisor;

	}


}

void intArrayReplace(int16_t *arraydestination, int16_t *arrayreplacer,uint16_t size_array ){
	for (int i = 0; i < size_array; ++i){
		arraydestination[i] = arrayreplacer[i];
	}
}



int16_t get_acc(uint8_t axis) {
	if(axis < NB_AXIS) {
		return imu_values.acc_raw[axis];
	}
	return 0;
}

void get_acc_all(int16_t *values) {
	values[X_AXIS] = imu_values.acc_raw[X_AXIS];
	values[Y_AXIS] = imu_values.acc_raw[Y_AXIS];
	values[Z_AXIS] = imu_values.acc_raw[Z_AXIS];
}


int16_t get_acc_offset(uint8_t axis) {
	if(axis < NB_AXIS) {
		return imu_values.acc_offset[axis];
	}
	return 0;
}

float get_acceleration(uint8_t axis) {
	if(axis < NB_AXIS) {
		return imu_values.acceleration[axis];
	}
	return 0;
}


int16_t get_gyro(uint8_t axis) {
	if(axis < NB_AXIS) {
		return imu_values.gyro_raw[axis];
	}
	return 0;
}

void get_gyro_all(int16_t *values) {
	values[X_AXIS] = imu_values.gyro_raw[X_AXIS];
	values[Y_AXIS] = imu_values.gyro_raw[Y_AXIS];
	values[Z_AXIS] = imu_values.gyro_raw[Z_AXIS];
}

int16_t get_gyro_offset(uint8_t axis) {
	if(axis < NB_AXIS) {
		return imu_values.gyro_offset[axis];
	}
	return 0;
}

float get_gyro_rate(uint8_t axis) {
	if(axis < NB_AXIS) {
		return imu_values.gyro_rate[axis];
	}
	return 0;
}

float get_temperature(void) {
	return imu_values.temperature;
}

/**************************END PUBLIC FUNCTIONS***********************************/

