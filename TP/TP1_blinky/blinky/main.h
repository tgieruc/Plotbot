#ifndef MAIN_H
#define MAIN_H

/* LEDs that can be used in this TP1
  LED1 			: GPIOD pin 5
  LED3 			: GPIOD pin 6
  LED5 			: GPIOD pin 10
  LED7 			: GPIOD pin 11
  FRONT_LED 	: GPIOD pin 14
WARNING : Not on the same port !!
  BODY_LED		: GPIOB pin 2
*/


#define LED1     	GPIOD, 5
#define LED3     	GPIOD, 6
#define LED5     	GPIOD, 10
#define LED7     	GPIOD, 11
#define FRONT_LED	GPIOD, 14
#define BODY_LED	GPIOB, 2

//TASK 1 4 ---------
#define LED_USED	LED7

//TASK 2 ---------
//#define LED_USED	FRONT_LED

//TASK 3 ---------
//#define LED_USED	BODY_LED



void wait_05sec(void);
void wait_1sec(void);
void pattern0(void);
void pattern1(void);


#endif /* MAIN_H_ */
