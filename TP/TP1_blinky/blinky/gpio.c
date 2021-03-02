#include <stm32f407xx.h>
#include <gpio.h>
#include <main.h>



// TASK 1,4 : opendrain : OTy = 1, floating : PUPDRy = 00
// TASK 2,3 : opendrain : OTy = 1, pull-up  : PUPDRy = 01
void gpio_config_output_opendrain_floating(GPIO_TypeDef *port, unsigned int pin)
{
    // Output type open-drain : OTy = 1
    port->OTYPER |= (1 << pin);

    // Output data low : ODRy = 0s
    port->ODR &= ~(0 << pin);

    // Floating, no pull-up/down : PUPDRy = 00
    port->PUPDR &= ~(3 << (pin * 2));

    // Output speed highest : OSPEEDRy = 11
    port->OSPEEDR |= (3 << (pin * 2));

    // Output mode : MODERy = 01
    port->MODER = (port->MODER & ~(3 << (pin * 2))) | (1 << (pin * 2));
}

void gpio_config_output_opendrain_pullup(GPIO_TypeDef *port, unsigned int pin)
{
    // Output type open-drain : OTy = 1
    port->OTYPER |= (1 << pin);

    // Output data low : ODRy = 0s
    port->ODR &= ~(0 << pin);

    // pull-up : PUPDRy = 01
    port->PUPDR = ((port->PUPDR & ~(3 << (pin * 2))) | (1 << (pin * 2)));

    // Output speed highest : OSPEEDRy = 11
    port->OSPEEDR |= (3 << (pin * 2));

    // Output mode : MODERy = 01
    port->MODER = (port->MODER & ~(3 << (pin * 2))) | (1 << (pin * 2));
}

void gpio_config_output_pushpull(GPIO_TypeDef *port, unsigned int pin)
{
    // Output type push-pull : OTy = 0
    port->OTYPER &= ~(1 << pin);

    // Output data low : ODRy = 0
    port->ODR &= ~(1 << pin);

    // Floating, no pull-up/down : PUPDRy = 00
    port->PUPDR &= ~(3 << (pin * 2));

    // Output speed highest : OSPEEDRy = 11
    port->OSPEEDR |= (3 << (pin * 2));

    // Output mode : MODERy = 01
    port->MODER = (port->MODER & ~(3 << (pin * 2))) | (1 << (pin * 2));
}

void gpio_config_input(GPIO_TypeDef *port, unsigned int pin)
{
	// Pull-down : PUPDRy = 10
	port->PUPDR = (port->PUPDR & ~(3 << (pin * 2))) | (2 << (pin * 2));

	// Output speed highest : OSPEEDRy = 11
	port->OSPEEDR |= (3 << (pin * 2));

	// Input mode : MODERy = 00
	port->MODER &= ~(3 << (pin * 2));
}

void gpio_set(GPIO_TypeDef *port, unsigned int pin)
{
    port->BSRR = (1 << pin);
}

void gpio_clear(GPIO_TypeDef *port, unsigned int pin)
{
    port->BSRR = (1 << (pin + 16));
}

void gpio_toggle(GPIO_TypeDef *port, unsigned int pin)
{
    if (port->ODR & (1<<pin)) {
        gpio_clear(port, pin);
    } else {
        gpio_set(port, pin);
    }
}

bool gpio_read(GPIO_TypeDef *port, unsigned int pin)
{
	return (port->IDR & (1<<pin));
}
