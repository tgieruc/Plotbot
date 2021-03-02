#include <stm32f4xx.h>
#include <system_clock_config.h>
#include <gpio.h>
#include <main.h>
#include <timer.h>
#include <selector.h>
#include <stdbool.h>


// Init function required by __libc_init_array
void _init(void) {}

const int led_used[4] = {5,6,10,11}; //task 4


int main(void)
{
    SystemClock_Config();


//// TASK 1
//// Enable GPIOD peripheral clock
//    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIODEN;
//// LED used init
//	gpio_config_output_opendrain_floating(LED7);
//    gpio_clear(LED7);
//
//    while(1){
//    	gpio_toggle(LED7);
//    	wait_1sec();
//    }
//
//// END TASK1

//---------------------------------------------------------

//// TASK 2
//// Enable GPIOD peripheral clock
//    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIODEN;
//// LED used init
//	gpio_config_output_opendrain_pullup(FRONT_LED);
//    gpio_clear(FRONT_LED);
//
//    while(1){
//    	gpio_toggle(FRONT_LED);
//    	wait_1sec();
//    }
//
//// END TASK 2

//---------------------------------------------------------

//// TASK 3
//// Enable GPIOB peripheral clock
//	RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOBEN;
//// LED used init
//	gpio_config_output_pushpull(BODY_LED); // TASK 1
//	gpio_clear(BODY_LED);
//
//	while(1){
//		gpio_toggle(BODY_LED);
//		wait_1sec();
//	}
//
//// END TASK 3

//---------------------------------------------------------

//// TASK 4
//    int selector, old_selector = 0;
//    bool do_pattern0 = true;
//// Enable GPIOB + GPIOD peripheral clock
//    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOCEN;
//// SELECTOR init
//    init_selector();
////	LED init
//    for (unsigned int i = 0; i < sizeof(led_used)/sizeof(led_used[0]); ++i) {
//        gpio_config_output_opendrain_pullup(GPIOD, led_used[i]);
//        gpio_set(GPIOD, led_used[i]);
//    }
//
//	while(1){
//		selector = get_selector();
//		if (selector != old_selector){
//			if (do_pattern0) do_pattern0 = false;
//			else do_pattern0 = true;
//
//		    for (unsigned int i = 0; i < sizeof(led_used)/sizeof(led_used[0]); ++i) {
//		        gpio_set(GPIOD, led_used[i]);
//		    }
//			old_selector = selector;
//		}
//
//		if (do_pattern0){
//			pattern0();
//		} else {
//			pattern1();
//		}
//	}
//// END TASK 4

//---------------------------------------------------------

//// TASK 5
//// Enable GPIOD peripheral clock
//    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIODEN;
//// LED used init
//	gpio_config_output_opendrain_floating(LED7);
//	gpio_clear(LED7);
//
//	timer7_start();
//
//	while(1){
//		;
//	}
//
//// END TASK1



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
