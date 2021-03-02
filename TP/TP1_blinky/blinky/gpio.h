#ifndef GPIO_H
#define GPIO_H

#include <stm32f407xx.h>
#include <stdbool.h>


void gpio_config_output_opendrain_floating(GPIO_TypeDef *port, unsigned int pin);
void gpio_config_output_opendrain_pullup(GPIO_TypeDef *port, unsigned int pin);
void gpio_config_output_pushpull(GPIO_TypeDef *port, unsigned int pin);

void gpio_config_input(GPIO_TypeDef *port, unsigned int pin);

void gpio_set(GPIO_TypeDef *port, unsigned int pin);
void gpio_clear(GPIO_TypeDef *port, unsigned int pin);
void gpio_toggle(GPIO_TypeDef *port, unsigned int pin);
bool gpio_read(GPIO_TypeDef *port, unsigned int pin);
#endif /* GPIO_H */
