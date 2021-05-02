#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <usbcfg.h>
#include <main.h>
#include <chprintf.h>
#include <motors.h>
#include <audio/microphone.h>
#include <audio/play_sound_file.h>
#include <audio/audio_thread.h>
#include <sdio.h>
#include <leds_animations.h>
//#include <leds.h>

#include <process_image.h>
#include <audio_processing.h>
#include <fft.h>
#include <arm_math.h>
#include "sensors/VL53L0X/VL53L0X.h"
#include "sensors/proximity.h"
#include "smartmove.h"
#include <camera/po8030.h>



void SendUint8ToComputer(uint8_t* data, uint16_t size)
{
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)"START", 5);
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)&size, sizeof(uint16_t));
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)data, size);
}

static void serial_start(void)
{
	static SerialConfig ser_cfg = {
	    115200,
	    0,
	    0,
	    0,
	};

	sdStart(&SD3, &ser_cfg); // UART3.
}

static void timer12_start(void){
    //General Purpose Timer configuration   
    //timer 12 is a 16 bit timer so we can measure time
    //to about 65ms with a 1Mhz counter
    static const GPTConfig gpt12cfg = {
        1000000,        /* 1MHz timer clock in order to measure uS.*/
        NULL,           /* Timer callback.*/
        0,
        0
    };

    gptStart(&GPTD12, &gpt12cfg);
    //let the timer count to max value
    gptStartContinuous(&GPTD12, 0xFFFF);
}


messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

int main(void)
{
	set_rgb_led(0, 0, 0, 10);
	set_rgb_led(1, 0, 0, 10);
	set_rgb_led(2, 0, 0, 10);
	set_rgb_led(3, 0, 0, 10);

    halInit();
    chSysInit();
    mpu_init();

    messagebus_init(&bus, &bus_lock, &bus_condvar);

    //starts the serial communication
    serial_start();
    //starts the USB communication
    usb_start();
    //starts timer 12
    timer12_start();
    //inits the motors
    motors_init();

    //inits the camera
    dcmi_start();
	po8030_start();

	//inits SD card
	sdio_start();

	//init dac
	dac_start();

	//inits the speaker
	playSoundFileStart();

	//inits the TOF
    VL53L0X_start();

    //inits the proximity sensors
    proximity_start();

    leds_animations_start();

    mic_start(&processAudioData);

    audioSeq_start();

	process_image_start();

    smartmove_start();


    /* Infinite loop. */
    while (1) {
        chThdSleepMilliseconds(1000);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
