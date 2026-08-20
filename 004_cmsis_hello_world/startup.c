// import the end-of-RAM address from linker script
extern unsigned int _estack; 

void Reset_Handler(void);
int main(void);

// vector table
__attribute__((section(".isr_vector")))
void (*const vector_table[])(void) = {
    (void (*)(void))(&_estack),
    Reset_Handler
};

void Reset_Handler(void) {
    // kick off
    main(); 
    while(1);
}