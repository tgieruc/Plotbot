#include <stm32f4xx.h>
#include <system_clock_config.h>
#include <gpio.h>
#include <main.h>

// Init function required by __libc_init_array
void _init(void) {}

int main(void)
{
    SystemClock_Config();

//    const int led_used[4] = {5,6,10,11}; //TASK 4
    // Enable GPIOD peripheral clock
//    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIODEN; // TASK 1,2,4
    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOBEN; // TASK 3

    // LED used init
//    TASK 1,2,3  --------------
    gpio_config_output_opendrain(LED_USED);
    gpio_clear(LED_USED);
// 	  -------------------

//    TASK 4
//    for (unsigned int i = 0; i < sizeof(led_used)/sizeof(led_used[0]); ++i) {
//        gpio_config_output_opendrain(GPIOD, led_used[i]);
//        gpio_clear(GPIOD, led_used[i]);
//    }




    while (1) {
//    	TASK 1,2,3 --------------------------
    	gpio_toggle(LED_USED);
        for (unsigned int i = 0; i < 168000000/20; ++i){
        	;
        }
//		---------------------------------

//		TASK 4 --------------------------
//        for (unsigned int i = 0; i < sizeof(led_used)/sizeof(led_used[0]); ++i) {
//            gpio_toggle(GPIOD, led_used[i]);
//        }
//
//		for (unsigned int i = 0; i < 168000000/20; ++i){
//			;
//		}
//		---------------------------------
    }
}
