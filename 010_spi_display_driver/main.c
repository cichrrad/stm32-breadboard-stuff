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
    dd_draw_circle(64,32,12,false);
    dd_fill_circle(64,32,8,false);
    dd_write_string(4,17,"Hello World!",false,false);
    dd_write_string(4,27,"ASDFG",false,true);
    dd_draw_triangle(72,17,96,3,125,60,false);
    dd_fill_triangle(10,10,15,3,20,10,false);

    dd_update();

    while(1){
        // Zzzzz
    }
    return 0;
}