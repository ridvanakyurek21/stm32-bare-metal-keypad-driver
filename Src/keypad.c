#include "keypad.h"

void keypad_init(const Keypad_Config_t *config)
{
    // Rows: input mode with pull-down

    // R0
    config->R0->MODER &= ~(3U << (config->pin_no_r0 * 2));
    config->R0->PUPDR &= ~(3U << (config->pin_no_r0 * 2));
    config->R0->PUPDR |=  (2U << (config->pin_no_r0 * 2));

    // R1
    config->R1->MODER &= ~(3U << (config->pin_no_r1 * 2));
    config->R1->PUPDR &= ~(3U << (config->pin_no_r1 * 2));
    config->R1->PUPDR |=  (2U << (config->pin_no_r1 * 2));

    // R2
    config->R2->MODER &= ~(3U << (config->pin_no_r2 * 2));
    config->R2->PUPDR &= ~(3U << (config->pin_no_r2 * 2));
    config->R2->PUPDR |=  (2U << (config->pin_no_r2 * 2));

    // R3
    config->R3->MODER &= ~(3U << (config->pin_no_r3 * 2));
    config->R3->PUPDR &= ~(3U << (config->pin_no_r3 * 2));
    config->R3->PUPDR |=  (2U << (config->pin_no_r3 * 2));

    // Columns: output mode with no pull

    // C0
    config->C0->MODER &= ~(3U << (config->pin_no_c0 * 2));
    config->C0->MODER |=  (1U << (config->pin_no_c0 * 2));
    config->C0->PUPDR &= ~(3U << (config->pin_no_c0 * 2));

    // C1
    config->C1->MODER &= ~(3U << (config->pin_no_c1 * 2));
    config->C1->MODER |=  (1U << (config->pin_no_c1 * 2));
    config->C1->PUPDR &= ~(3U << (config->pin_no_c1 * 2));

    // C2
    config->C2->MODER &= ~(3U << (config->pin_no_c2 * 2));
    config->C2->MODER |=  (1U << (config->pin_no_c2 * 2));
    config->C2->PUPDR &= ~(3U << (config->pin_no_c2 * 2));

    // C3
    config->C3->MODER &= ~(3U << (config->pin_no_c3 * 2));
    config->C3->MODER |=  (1U << (config->pin_no_c3 * 2));
    config->C3->PUPDR &= ~(3U << (config->pin_no_c3 * 2));

    config->C0->ODR &= ~(1U << config->pin_no_c0);
    config->C1->ODR &= ~(1U << config->pin_no_c1);
    config->C2->ODR &= ~(1U << config->pin_no_c2);
    config->C3->ODR &= ~(1U << config->pin_no_c3);
}

static void keypad_debounce_delay(void)
{
    for (volatile uint32_t i = 0; i < 30000; i++)
    {
        __asm volatile ("nop");
    }
}

static int keypad_scan_rows(const Keypad_Config_t *config)
{
    if (config->R0->IDR & (1U << config->pin_no_r0))
    {
        keypad_debounce_delay();

        if (config->R0->IDR & (1U << config->pin_no_r0))
            return 0;
    }

    if (config->R1->IDR & (1U << config->pin_no_r1))
    {
        keypad_debounce_delay();

        if (config->R1->IDR & (1U << config->pin_no_r1))
            return 1;
    }

    if (config->R2->IDR & (1U << config->pin_no_r2))
    {
        keypad_debounce_delay();

        if (config->R2->IDR & (1U << config->pin_no_r2))
            return 2;
    }

    if (config->R3->IDR & (1U << config->pin_no_r3))
    {
        keypad_debounce_delay();

        if (config->R3->IDR & (1U << config->pin_no_r3))
            return 3;
    }

    return -1;
}

char keypad_get_pressed_key(const Keypad_Config_t *config)
{
    int row_no;

    // Scan column C0
    config->C0->ODR |= (1U << config->pin_no_c0);
    row_no = keypad_scan_rows(config);

    if (row_no != -1)
    {
        config->C0->ODR &= ~(1U << config->pin_no_c0);
        return config->key_map[row_no][0];
    }

    config->C0->ODR &= ~(1U << config->pin_no_c0);

    // Scan column C1
    config->C1->ODR |= (1U << config->pin_no_c1);
    row_no = keypad_scan_rows(config);

    if (row_no != -1)
    {
        config->C1->ODR &= ~(1U << config->pin_no_c1);
        return config->key_map[row_no][1];
    }

    config->C1->ODR &= ~(1U << config->pin_no_c1);

    // Scan column C2
    config->C2->ODR |= (1U << config->pin_no_c2);
    row_no = keypad_scan_rows(config);

    if (row_no != -1)
    {
        config->C2->ODR &= ~(1U << config->pin_no_c2);
        return config->key_map[row_no][2];
    }

    config->C2->ODR &= ~(1U << config->pin_no_c2);

    // Scan column C3
    config->C3->ODR |= (1U << config->pin_no_c3);
    row_no = keypad_scan_rows(config);

    if (row_no != -1)
    {
        config->C3->ODR &= ~(1U << config->pin_no_c3);
        return config->key_map[row_no][3];
    }

    config->C3->ODR &= ~(1U << config->pin_no_c3);

    return '\0';
}
