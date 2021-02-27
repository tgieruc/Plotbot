#include <stm32f4xx.h>
#include <system_clock_config.h>
#include <gpio.h>
#include <main.h>
#include <timer.h>

// Init function required by __libc_init_array
void _init(void) {}

const int led_used[4] = {5,6,10,11}; //task 4


int main(void)
{
    SystemClock_Config();


    // Enable GPIOD peripheral clock
    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIODEN; // TASK 1,2,5
//    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOBEN; // TASK 3
//    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIODEN; // TASK 4
//    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOCEN; // TASK 4

    // LED used init
//    TASK 1,2,3,5  --------------
    gpio_config_output_opendrain(LED_USED);
    gpio_clear(LED_USED);
// 	  -------------------

//    TASK 4
//    gpio_config_input(GPIOC,13);
//    gpio_config_input(GPIOC,14);
//    gpio_config_input(GPIOC,15);

//    for (unsigned int i = 0; i < sizeof(led_used)/sizeof(led_used[0]); ++i) {
//        gpio_config_output_opendrain(GPIOD, led_used[i]);
//        gpio_set(GPIOD, led_used[i]);
//    }




    while (1) {
//    	TASK  5 6
    	timer7_start();
//    	TASK 1,2,3 --------------------------
//    	gpio_toggle(LED_USED);
//        for (unsigned int i = 0; i < 168000000/20; ++i){
//        	;
//        }
//		---------------------------------

//		TASK 4 --------------------------
    	//TODO: task 4 a finir : utilisation GPIOC input p.283 STM32F407 Data Sheet p 11 12 TP
//    	pattern1();
//		---------------------------------




    }
}


void wait_05sec(){
	for (unsigned int i = 0; i < 168000000/40; ++i){
		;
	}
}

void wait_1sec(){
	for (unsigned int i = 0; i < 168000000/20; ++i){
		;
	}
}

void pattern0(){
	for (unsigned int i = 0; i < sizeof(led_used)/sizeof(led_used[0]); ++i) {
		gpio_toggle(GPIOD, led_used[i]);
		wait_1sec();
	}
}

void pattern1(){
	for (unsigned int i = 0; i < sizeof(led_used)/sizeof(led_used[0]); ++i) {
			gpio_toggle(GPIOD, led_used[i]);
			wait_05sec();
			gpio_toggle(GPIOD, led_used[i]);
			wait_05sec();
		}
}
