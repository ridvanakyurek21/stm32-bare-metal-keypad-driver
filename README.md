# STM32 Bare-Metal 4x4 Keypad Driver

A bare-metal 4x4 matrix keypad driver developed for the STM32F407 microcontroller.

This project was not created only to make a keypad work. The main goal was to understand the STM32 GPIO system, GPIO registers, electrical behavior of the pins, and matrix keypad scanning as deeply as possible by implementing the driver at register level.

No STM32 HAL keypad driver or ready-made keypad library was used. GPIO configuration is performed directly through memory-mapped registers.

---

## Demo

Project demo video:

[▶️ Keypad Driver Demo](https://github.com/user-attachments/assets/5850e141-4ebe-4301-9232-0bcd85fcacc9)

The video shows:




- The physical keypad circuit.
- Individual key presses.
- Detected characters printed to the terminal through ITM.
- Several keys being held down.
- A held key being reported only once instead of continuously.

---

## Project Goals

The main goal of this project was to understand the STM32 GPIO system by working with it directly at register level.

The project focuses on:

- GPIO register structure
- Memory-mapped peripheral access
- `volatile`
- `MODER`
- `PUPDR`
- `IDR`
- `ODR`
- GPIO input/output behavior
- Pull-down resistor behavior
- Matrix keypad scanning
- Debouncing
- Driver/application separation
- Configuration-based driver design
- ITM debug output

The project uses direct register access instead of STM32 HAL or a ready-made keypad driver.

---

# 4x4 Matrix Keypad

A 4x4 matrix keypad uses 8 GPIO pins:

- 4 rows
- 4 columns

The key layout is:

```text
        C0   C1   C2   C3

R0      1    2    3    A
R1      4    5    6    B
R2      7    8    9    C
R3      *    0    #    D
```

Each key provides an electrical connection between one row and one column.

This allows 16 keys to be detected using only 8 GPIO pins.

---

# GPIO Configuration and Circuit Design

The keypad scanning logic was designed by considering both the software configuration and the electrical behavior of the GPIO pins.

Row and column pins have different roles, so they are configured differently.

## Row Pins

The row pins are configured as:

```text
MODER = 00  -> Input
PUPDR = 10  -> Pull-down
```

The pull-down resistors keep the row inputs at a defined LOW level when no key is pressed.

Therefore, under normal conditions:

```text
Key released -> Row = 0
```

When a column is driven HIGH and the corresponding key is pressed, the key creates an electrical path between the column and the row.

Simplified circuit:

```text
       GPIO Output
           |
          HIGH
           |
        Column
           |
         [Key]
           |
          Row
           |
       Pull-down
           |
          GND
```

When the key is pressed, the HIGH level from the active column reaches the row input.

Therefore:

```text
Key released -> Row = 0
Key pressed  -> Row = 1
```

The pull-down resistor provides the default LOW state for the row input.

---

## Column Pins

The column pins are configured as:

```text
MODER = 01  -> Output
PUPDR = 00  -> No pull-up / pull-down
```

`PUPDR = 00` is used because the columns are outputs and their logic level is controlled through the `ODR` register.

For example:

```text
C0 = HIGH
C1 = LOW
C2 = LOW
C3 = LOW
```

Only C0 is active during this scan step.

The driver then reads the four row inputs.

After that, C0 is driven LOW and C1 is driven HIGH. The same process continues for all columns.

---

# Keypad Scanning

The driver activates one column at a time and reads the row inputs.

For example:

```text
C1 = HIGH
R1 = HIGH
```

means that the key at row 1 and column 1 is pressed:

```text
Key = 5
```

The complete scan sequence is:

```text
C0 -> HIGH -> Read rows -> LOW
C1 -> HIGH -> Read rows -> LOW
C2 -> HIGH -> Read rows -> LOW
C3 -> HIGH -> Read rows -> LOW
```

This sequence is continuously repeated.

---

# Key Mapping

The physical row/column position is converted to a character through a software key map.

```c
.key_map =
{
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
}
```

For example:

```text
row = 1
column = 2
```

results in:

```text
key_map[1][2] = '6'
```

This keeps the physical scanning logic separate from the character mapping used by the application.

---

# Problem Encountered: Key Bounce

During the first tests, the keypad worked, but a single physical key press could sometimes be detected more than once.

For example:

```text
5
5
```

could appear for a single press.

After investigating the problem, it was identified as **contact bounce**, a common behavior of mechanical switches.

When a mechanical key is pressed, the electrical signal does not always become stable immediately. For a short period, it can transition several times:

```text
1 -> 0 -> 1 -> 0 -> 1
```

Without debouncing, these transitions can be interpreted as multiple key presses.

---

# Debounce Solution

A simple debounce mechanism was added to the driver.

When a row is first detected HIGH, the driver waits for a short period and then reads the same input again.

```text
HIGH detected
      |
      v
 short delay
      |
      v
 read again
      |
      +---- HIGH -> valid key press
      |
      +---- LOW  -> ignore
```

This filters short transitions caused by mechanical contact bounce.

The application also keeps track of the previously detected key:

```c
if (pressed_key != '\0' && pressed_key != previous_key)
{
    printf("Key pressed: %c\r\n", pressed_key);
}
```

This prevents the same key from being printed continuously while it is being held down.

The demo video includes held-key tests where a key remains pressed but is reported only once.

---

# Driver Structure

The driver receives its hardware configuration through `Keypad_Config_t`.

The configuration contains:

- Row GPIO ports
- Row pin numbers
- Column GPIO ports
- Column pin numbers
- Key map

The main API is:

```c
void keypad_init(const Keypad_Config_t *config);

char keypad_get_pressed_key(const Keypad_Config_t *config);
```

`keypad_init()` configures the required GPIO registers.

`keypad_get_pressed_key()` scans the keypad and returns the detected key as a character.

This makes the driver independent from one specific physical GPIO arrangement.

---

# GPIO Registers Used

The driver mainly uses the following GPIO registers:

| Register | Purpose |
|----------|---------|
| `MODER` | Selects the GPIO mode |
| `PUPDR` | Configures pull-up / pull-down resistors |
| `IDR` | Reads the input level |
| `ODR` | Controls the output level |

Basic flow:

```text
MODER
  |
  +--> Select input / output mode

PUPDR
  |
  +--> Select pull-up / pull-down

ODR
  |
  +--> Drive column HIGH / LOW

IDR
  |
  +--> Read row state
```

---

# Register-Level Implementation

The GPIO register layout is represented in C using a structure:

```c
typedef struct
{
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
```

The structure allows the GPIO peripheral registers to be accessed directly through their memory-mapped addresses.

The purpose of this approach is to understand how the GPIO peripheral works below the abstraction level provided by a HAL.

---

# Debug

The STM32 Cortex-M ITM debug mechanism is used to observe key detection results.

`printf()` output is sent through ITM to the debug terminal.

Example output:

```text
--- KEYPAD DRIVER STARTED ---
Key pressed: 1
Key pressed: 2
Key pressed: 3
Key pressed: A
Key pressed: 5
Key pressed: 6
Key pressed: #
```

The `ITM_SendChar()` routine is based on standard ARM Cortex-M ITM usage examples/documentation and was not written from scratch by me.

---

# Hardware

The project was developed on an STM32F407-based board.

Keypad:

```text
4x4 Matrix Keypad
```

## Row Connections

```text
R0 -> PE7
R1 -> PE9
R2 -> PE11
R3 -> PE13
```

## Column Connections

```text
C0 -> PE15
C1 -> PB11
C2 -> PB13
C3 -> PB15
```

---

# Project Structure

```text
stm32-bare-metal-keypad-driver/
│
├── Inc/
│   └── keypad.h
│
├── Src/
│   ├── keypad.c
│   └── main.c
│
└── README.md
```

---

# What I Learned

This project was not only about reading a keypad. It was also a practical study of the GPIO architecture and its electrical behavior.

Main topics covered:

- Memory-mapped registers
- GPIO register access
- `volatile`
- GPIO mode configuration
- Input / output behavior
- Pull-down resistors
- Driving outputs with `ODR`
- Reading inputs with `IDR`
- Matrix keypad scanning
- Mechanical switch bounce
- Debouncing
- Driver / application separation
- Configuration-based hardware mapping
- ITM debug output

---

# Conclusion

This project was primarily a learning exercise for understanding the STM32 GPIO system at register level.

Instead of using a ready-made keypad library, I implemented the GPIO configuration, matrix scanning, key mapping, and debounce logic while focusing on how the underlying electrical and register-level mechanisms work.

A major part of the project was understanding why the row inputs use pull-down resistors, why the columns are driven one at a time, how a key press creates the electrical path from an active column to a row input, and how that physical event becomes a character in software.

The contact bounce problem was also encountered on real hardware, investigated, and addressed with a debounce mechanism.

The current version successfully performs keypad scanning, debouncing, key mapping, and ITM-based debug output.
