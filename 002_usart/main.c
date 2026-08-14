#include <stdint.h>

// mem layout from RM0440
// Peripheral Base Addresses
#define GPIOA_BASE 0x48000000U  // Port A GPIO (holds LED LD2 on pin 5 - see 001)
#define USART2_BASE 0x40004400U // USART2 connected to ST-link
#define RCC_BASE 0x40021000U    // Reset & Clock control

// STRUCT OVERLAYS
// NOTE -- This is a bit of an overkill here,
// but its cool and exactly what is in generated
// HAL

// GPIO
// (ORDER MATTERS - to match variables with layout from RM0440)
typedef struct
{
    volatile uint32_t MODER;   // Offset: 0x00
    volatile uint32_t OTYPER;  // Offset: 0x04
    volatile uint32_t OSPEEDR; // ...
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFRL; // ...
    volatile uint32_t AFRH; // Offset: 0x24
    volatile uint32_t BRR;  // Offset: 0x28
} GPIO_TypeDef;

// USART
typedef struct
{
    volatile uint32_t CR1; // Offset: 0x00
    volatile uint32_t CR2; // Offset: 0x04
    volatile uint32_t CR3; // ...
    volatile uint32_t BRR;
    volatile uint32_t GTPR;
    volatile uint32_t RTOR;
    volatile uint32_t RQR;
    volatile uint32_t ISR;
    volatile uint32_t ICR; // ...
    volatile uint32_t RDR; // Offset: 0x24 (Receive Data Register)
    volatile uint32_t TDR; // Offset: 0x28 (Transmit Data Register)
} USART_TypeDef;

// RCC
typedef struct
{
    volatile uint32_t CR;    // Offset: 0x00
    volatile uint32_t ICSCR; // Offset: 0x04
    volatile uint32_t CFGR;
    volatile uint32_t PLLCFGR;

    uint32_t RESERVED_0[2]; // To not break alignment

    volatile uint32_t CIER;
    volatile uint32_t CIFR;
    volatile uint32_t CICR;

    uint32_t RESERVED_1;

    volatile uint32_t AHB1RSTR;
    volatile uint32_t AHB2RSTR;
    volatile uint32_t AHB3RSTR;

    uint32_t RESERVED_2;

    volatile uint32_t APB1RSTR1;
    volatile uint32_t APB1RSTR2;
    volatile uint32_t APB2RSTR;

    uint32_t RESERVED_3;

    volatile uint32_t AHB1ENR;
    volatile uint32_t AHB2ENR;
    volatile uint32_t AHB3ENR;

    uint32_t RESERVED_4;

    volatile uint32_t APB1ENR1;
    volatile uint32_t APB1ENR2;
    volatile uint32_t APB2ENR;

    uint32_t RESERVED_5;

    volatile uint32_t AHB1SMENR;
    volatile uint32_t AHB2SMENR;
    volatile uint32_t AHB3SMENR;

    uint32_t RESERVED_6;

    volatile uint32_t APB1SMENR1;
    volatile uint32_t APB1SMENR2;
    volatile uint32_t APB2SMENR;

    uint32_t RESERVED_7;

    volatile uint32_t CCIPR;

    uint32_t RESERVED_8;

    volatile uint32_t BDCR;
    volatile uint32_t CSR;
    volatile uint32_t CRRCR;  // Offset: 0x98
    volatile uint32_t CCIPR2; // Offset: 0x9C

} RCC_TypeDef;

// cast pointers of bases as respective
// structs -> overlay of variables over the registers
#define GPIOA ((GPIO_TypeDef *)GPIOA_BASE)
#define USART2 ((USART_TypeDef *)USART2_BASE)
#define RCC ((RCC_TypeDef *)RCC_BASE)

// Enable macros
#define USART_CR1_UE (1 << 0)   // USART Enable
#define USART_CR1_RE (1 << 2)   // Receiver Enable
#define USART_CR1_TE (1 << 3)   // Transmitter Enable
#define USART_ISR_RXNE (1 << 5) // Read Data Register Not Empty
#define USART_ISR_TXE (1 << 7)  // Transmit Data Register Empty

#define RCC_AHB2ENR_GPIOAE (1 << 0)    // GPIOA clock enable
#define RCC_APB1ENR1_USART2E (1 << 17) // USART2 clock enable

void uart2_init(void)
{
    // Enable clocks for GPIOA and USART2
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAE;
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2E;

    // Configure Port A pin 2 (TX) and Port A pin 3 (RX) for Alternate Function mode
    // pin 2 mode is set with pins [5,4], pin 3 with [7,6]
    GPIOA->MODER &= ~((3 << 4) | (3 << 6)); // clear [7..4]
    GPIOA->MODER |= ((2 << 4) | (2 << 6));  // write '1,0' to each

    // Set Alternate Function 7 (AF7 - serial comms) for pins 2,3
    // pin 2 mode is bits [11..8], pin 3 mode is bits [15..12] in AFRL.
    GPIOA->AFRL &= ~((0xF << 8) | (0xF << 12)); // clear them
    GPIOA->AFRL |= ((7 << 8) | (7 << 12));      // literally set them to 7

    // Using 16MHz system clock and targetting 115'200 baud rate
    // 16'000'000/115'200 ~ 139
    USART2->BRR = 139;

    // Enable USART2 TX and RX
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void uart2_write_char(char c)
{
    // Poll till we can write into TDR register for send-off
    while (!(USART2->ISR & USART_ISR_TXE)){};
    USART2->TDR = c;
}

char uart2_read_char(void)
{
    // Poll till we can read from RDR register
    while (!(USART2->ISR & USART_ISR_RXNE)){};
    return (char)USART2->RDR;
}

void uart2_write_string(const char *str)
{
    while (*str)
    {
        uart2_write_char(*str++);
    }
}

int main(void)
{
    uart2_init();

    // LED setup -- configure port A pin 5 to output mode ('01' on bits [11,10])
    GPIOA->MODER &= ~(3 << 10);
    GPIOA->MODER |= (1 << 10);

    uart2_write_string("==============================\r\n");
    uart2_write_string("Press '1' to turn ON the LED.\r\n");
    uart2_write_string("Press '0' to turn OFF the LED.\r\n");
    uart2_write_string("==============================\r\n");

    while (1)
    {
        // Poll for char
        char input = uart2_read_char();

        if (input == '1')
        {
            GPIOA->ODR |= (1 << 5); // LD2 ON
            uart2_write_string("LED ON\r\n> ");
        }
        else if (input == '0')
        {
            GPIOA->ODR &= ~(1 << 5); // LD2 OFF
            uart2_write_string("LED OFF\r\n> ");
        }
        else
        {
            // Are you dense?
            uart2_write_string("==============================\r\n");
            uart2_write_string("Press '1' to turn ON the LED.\r\n");
            uart2_write_string("Press '0' to turn OFF the LED.\r\n");
            uart2_write_string("==============================\r\n");
        }
    }

    return 0;
}