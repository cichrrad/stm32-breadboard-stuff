#include "stm32g491xx.h"
#include "device_drivers/display_driver/ddriver.h"
#include "device_drivers/tick_engine/systick_timer.h"
#include "miky_bitmap.h"

#include <stdint.h>

int main(void){

    SysTick_Init(16000000);
    DD_Init();
    dd_fill_rect(0,0,DD_WIDTH,DD_HEIGHT,true);
    dd_draw_bitmap(28,16,MIKY_WIDTH,MIKY_HEIGHT,miky,true);

    dd_update();

    while(1){
        // Zzzzz
    }
    return 0;
}