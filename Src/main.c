#include <stdint.h>
#include <stdio.h>
#include "keypad.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

#define GPIOE ((GPIO_TypeDef *)0x40021000)
#define GPIOB ((GPIO_TypeDef *)0x40020400)
#define RCC_AHB1ENR (*(volatile uint32_t *)0x40023830)

#define DEMCR              (*(volatile uint32_t *)0xE000EDFCU)
#define ITM_STIMULUS_PORT0 (*(volatile uint32_t *)0xE0000000U)
#define ITM_TRACE_EN       (*(volatile uint32_t *)0xE0000E00U)

// Standard ITM output routine based on ARM Cortex-M documentation/examples.
// This implementation was not written by me.
void ITM_SendChar(uint8_t ch)
{
    // Enable trace
    DEMCR |= (1U << 24);

    // Enable stimulus port 0
    ITM_TRACE_EN |= (1U << 0);

    // Wait until the stimulus port is ready
    while (!(ITM_STIMULUS_PORT0 & 1U));

    // Send character
    ITM_STIMULUS_PORT0 = ch;
}

int main(void)
{
    // Enable GPIO clocks
    RCC_AHB1ENR |= (1U << 1);   // GPIOB
    RCC_AHB1ENR |= (1U << 4);   // GPIOE

    const Keypad_Config_t keypad_config =
    {
        // Rows
        .R0 = GPIOE, .pin_no_r0 = 7,
        .R1 = GPIOE, .pin_no_r1 = 9,
        .R2 = GPIOE, .pin_no_r2 = 11,
        .R3 = GPIOE, .pin_no_r3 = 13,

        // Columns
        .C0 = GPIOE, .pin_no_c0 = 15,
        .C1 = GPIOB, .pin_no_c1 = 11,
        .C2 = GPIOB, .pin_no_c2 = 13,
        .C3 = GPIOB, .pin_no_c3 = 15,

        // Key mapping
        .key_map =
        {
            {'1', '2', '3', 'A'},
            {'4', '5', '6', 'B'},
            {'7', '8', '9', 'C'},
            {'*', '0', '#', 'D'}
        }
    };

    keypad_init(&keypad_config);

    printf("\r\n--- KEYPAD DRIVER STARTED ---\r\n");

    char previous_key = '\0';

    for (;;)
    {
        char pressed_key = keypad_get_pressed_key(&keypad_config);

        if (pressed_key != '\0' && pressed_key != previous_key)
        {
            printf("Key pressed: %c\r\n", pressed_key);
        }

        previous_key = pressed_key;
    }
}
