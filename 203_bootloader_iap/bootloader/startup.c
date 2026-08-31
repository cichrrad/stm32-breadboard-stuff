extern unsigned int _estack;
extern unsigned int _sidata;
extern unsigned int _sdata;
extern unsigned int _edata;
extern unsigned int _sbss;
extern unsigned int _ebss;

void Reset_Handler(void);
void SysTick_Handler(void);
void DMA1_Channel1_IRQHandler(void);

int main(void);

// The Cortex-M vector table. Indexes 1-15 are core exceptions.
__attribute__((section(".isr_vector")))
void (*const vector_table[])(void) = {
    (void (*)(void))(&_estack), // 0: Initial Stack Pointer
    Reset_Handler,              // 1: Reset vector
    0,                          // 2: NMI
    0,                          // 3: Hard Fault
    0,                          // 4: Memory Management Fault
    0,                          // 5: Bus Fault
    0,                          // 6: Usage Fault
    0, 0, 0, 0,                 // 7-10: Reserved
    0,                          // 11: SVCall
    0,                          // 12: Debug Monitor
    0,                          // 13: Reserved
    0,                          // 14: PendSV
    SysTick_Handler,            // 15: SysTick Timer
    
    // --- Vendor Specific Interrupts (IRQs) ---
    0,                          // 16: IRQ0 (WWDG)
    0,                          // 17: IRQ1 (PVD_PVM)
    0,                          // 18: IRQ2 (RTC_TAMP_LSECSS)
    0,                          // 19: IRQ3 (RTC_WKUP)
    0,                          // 20: IRQ4 (FLASH)
    0,                          // 21: IRQ5 (RCC)
    0,                          // 22: IRQ6 (EXTI Line 0)
    0,
    0,
    0,
    0,
    DMA1_Channel1_IRQHandler
};

void Reset_Handler(void) {
    // Copy .data section from FLASH to RAM
    unsigned int *data_rom = &_sidata;
    unsigned int *data_ram = &_sdata;
    while (data_ram < &_edata) {
        *data_ram++ = *data_rom++;
    }

    // Zero out the .bss section
    unsigned int *bss_ram = &_sbss;
    while (bss_ram < &_ebss) {
        *bss_ram++ = 0;
    }

    // Kick off the main application
    main(); 
    while(1);
}


