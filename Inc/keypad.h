#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>

typedef struct
{
    // GPIO register map
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFRL;
    volatile uint32_t AFRH;

} GPIO_TypeDef;

typedef struct
{
    // Rows
    volatile GPIO_TypeDef *R0;
    uint8_t pin_no_r0;

    volatile GPIO_TypeDef *R1;
    uint8_t pin_no_r1;

    volatile GPIO_TypeDef *R2;
    uint8_t pin_no_r2;

    volatile GPIO_TypeDef *R3;
    uint8_t pin_no_r3;

    // Columns
    volatile GPIO_TypeDef *C0;
    uint8_t pin_no_c0;

    volatile GPIO_TypeDef *C1;
    uint8_t pin_no_c1;

    volatile GPIO_TypeDef *C2;
    uint8_t pin_no_c2;

    volatile GPIO_TypeDef *C3;
    uint8_t pin_no_c3;

    // Key mapping
    char key_map[4][4];

} Keypad_Config_t;

void keypad_init(const Keypad_Config_t *config);
char keypad_get_pressed_key(const Keypad_Config_t *config);

#endif
