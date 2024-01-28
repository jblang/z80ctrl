#ifndef HARDWARE_GPIO_H
#define HARDWARE_GPIO_H

#if defined(__AVR_AVR128DB28__)
#include "gpio_db28.h"
#elif defined(__AVR_AVR128DB64__)
#include "gpio_db64.h"
#else
#include "gpio_1284p.h"
#endif

#endif