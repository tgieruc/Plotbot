#include <stm32f4xx.h>
#include <system_clock_config.h>
#include <gpio.h>
#include <main.h>
#include <timer.h>
#include <motor.h>
#include <selector.h>

#define PI                  3.1415926536f
//TO ADJUST IF NECESSARY. NOT ALL THE E-PUCK2 HAVE EXACTLY THE SAME WHEEL DISTANCE
#define WHEEL_DISTANCE      5.35f    //cm
#define PERIMETER_EPUCK     (PI * WHEEL_DISTANCE)

// Init function required by __libc_init_array
void _init(void) {}

// Simple delay function
void delay(unsigned int n)
{
    while (n--) {
        __asm__ volatile ("nop");
    }
}


int main(void)
{
    SystemClock_Config();

    // Enable GPIOD GPIOB and GPIOE peripheral clock
    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOEEN;
    gpio_config_output_open_drain_floating(LED7);
    gpio_set(LED7);

//    // TASK 1-11
//    gpio_config_output_af_pushpull(FRONT_LED,2);
//    timer4_start();
//    // ---------
    motor_init();
    motor_set_position(10,10,13,13);
    wait_motor_done();
    motor_set_position(-13,13,13,13);
    wait_motor_done();
    motor_set_position(10,10,13,13);


    while (1) {
        
    }
}

