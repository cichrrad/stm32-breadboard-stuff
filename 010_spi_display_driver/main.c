#include "stm32g491xx.h"
#include "device_drivers/display_driver/ddriver.h"
#include "device_drivers/tick_engine/systick_timer.h"
#include "miky_bitmap.h"


#include <stdint.h>

int main(void){

    SysTick_Init(16000000);
    DD_Init();
    dd_fill_rect(0,0,DD_WIDTH,DD_HEIGHT,true);

    dd_draw_bitmap(0,16,MIKY_WIDTH,MIKY_HEIGHT,miky,true); 
    dd_draw_circle(80,32,12,false);
    dd_fill_circle(80,32,8,false);
    dd_write_string(30,56,"Hello World!",false,true);
    dd_draw_triangle(72,17,96,3,125,60,false);
    dd_fill_triangle(10,15,20,3,64,15,false);
    dd_draw_rect(112,4,10,20,false);
    dd_update();


    while(1){
        // Zzzzz
    }
    return 0;
}