# Notes

- This program shows tiny demo for using the display driver in FreeRTOS. It is very much similar to the bare metal driver, as it is adapted from it, BUT there are changes in place to align with RTOS practices -- specifically, removing blocking while loop waiting for the very last bit to be send during transmittion -- in bare-metal variant, this was in the interrupt. 
> (albeit I dont think it is that much of a war crime, because this interrupt only fires once last transaction fires, so this blocking loop is really short and we need to wait for the transfer to fully complete, but blocking inside IRQ, which is outside of FreeRTOS ability cheats the scheduler)

In this version, it is in `dd_update`.  Furthermore, driver was rewritten to notify the task it is running once transfer finishes via

```c
vTaskNotifyGiveFromISR(xRenderTaskHandle, &xHigherPriorityTaskWoken);
portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
```
I am on the fence regarding this decision, as it means you cannot use simple notify from other tasks to the task doing the rendering (hence semaphore usage in `204` for instance).