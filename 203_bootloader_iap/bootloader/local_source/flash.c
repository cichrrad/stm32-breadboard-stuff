#include "flash.h"

// Define the starting page of the app. 
// Flash starts at 0x08000000. App is at 0x08008000.
// 0x8000 / 2048 bytes per page = Page 16.
#define APP_START_PAGE 16 
#define APP_ADDRESS 0x08008000

bool Flash_Write_App(uint8_t *data, uint32_t length) {
    // Wait for flash to be ready
    while ((FLASH->SR & FLASH_SR_BSY) != 0);

    // Unlock the Flash Control Register
    if ((FLASH->CR & FLASH_CR_LOCK) != 0) {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
    
    // Check if unlock failed
    if ((FLASH->CR & FLASH_CR_LOCK) != 0) {
        return false; 
    }

    // Clear any previous error flags
    FLASH->SR = FLASH_SR_EOP | FLASH_SR_PROGERR | FLASH_SR_WRPERR | 
                FLASH_SR_PGAERR | FLASH_SR_SIZERR | FLASH_SR_PGSERR | 
                FLASH_SR_MISERR | FLASH_SR_FASTERR | FLASH_SR_OPTVERR;

    // ERASE PAGES
    // Calculate how many 2KB pages we need to wipe for this specific .bin
    uint32_t pages_to_erase = (length + 2047) / 2048; 

    for (uint32_t i = 0; i < pages_to_erase; i++) {
        uint32_t page = APP_START_PAGE + i;
        
        FLASH->CR &= ~FLASH_CR_PNB;                  // Clear page number
        FLASH->CR |= (page << FLASH_CR_PNB_Pos);     // Set target page
        FLASH->CR |= FLASH_CR_PER;                   // Set Page Erase bit
        FLASH->CR |= FLASH_CR_STRT;                  // Start erase
        
        while ((FLASH->SR & FLASH_SR_BSY) != 0);     // Wait for erase to finish
        
        if (FLASH->SR & FLASH_SR_EOP) {
            FLASH->SR = FLASH_SR_EOP;                // Clear End Of Operation flag
        }
    }
    FLASH->CR &= ~FLASH_CR_PER;                      // Disable Page Erase

    // WRITE DATA (64-bit Double Words)
    FLASH->CR |= FLASH_CR_PG;                        // Enable Programming

    uint32_t *src = (uint32_t *)data;                // Cast buffer to 32-bit words
    volatile uint32_t *dst = (volatile uint32_t *)APP_ADDRESS; 
    
    // Calculate total 64-bit chunks (round up)
    uint32_t double_words = (length + 7) / 8;

    for (uint32_t i = 0; i < double_words; i++) {
        // STM32G4 expects two 32-bit writes in sequence to make a 64-bit write
        dst[0] = src[0];
        dst[1] = src[1];
        
        while ((FLASH->SR & FLASH_SR_BSY) != 0);     // Wait for write to finish
        
        if (FLASH->SR & FLASH_SR_EOP) {
            FLASH->SR = FLASH_SR_EOP;                // Clear End Of Operation flag
        }

        dst += 2; // Advance destination by 8 bytes (two 32-bit words)
        src += 2; // Advance source by 8 bytes (two 32-bit words)
    }

    FLASH->CR &= ~FLASH_CR_PG;                       // Disable Programming

    // 5. Re-lock the flash to protect it
    FLASH->CR |= FLASH_CR_LOCK;

    return true;
}