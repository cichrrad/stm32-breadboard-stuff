#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* System Clock and Tick Setup */
#define configCPU_CLOCK_HZ                    ( 16000000 ) /* Internal clock */
#define configTICK_RATE_HZ                    ( ( TickType_t ) 1000 ) /* 1ms tick */

/* Basic OS Features */
#define configUSE_PREEMPTION                  1
#define configUSE_IDLE_HOOK                   0
#define configUSE_TICK_HOOK                   0
#define configMAX_PRIORITIES                  ( 5 )
#define configMINIMAL_STACK_SIZE              ( ( uint16_t ) 128 ) /* In words, not bytes */
#define configTOTAL_HEAP_SIZE                 ( ( size_t ) ( 10 * 1024 ) ) /* 10KB Heap */
#define configMAX_TASK_NAME_LEN               ( 16 )
#define configUSE_16_BIT_TICKS                0
#define configUSE_MUTEXES                     1

/* Memory Allocation */
#define configSUPPORT_DYNAMIC_ALLOCATION      1
#define configSUPPORT_STATIC_ALLOCATION       0

/* Timers (Required since you included timers.c) */
#define configUSE_TIMERS                      1
#define configTIMER_TASK_PRIORITY             ( 2 )
#define configTIMER_QUEUE_LENGTH              10
#define configTIMER_TASK_STACK_DEPTH          configMINIMAL_STACK_SIZE


/* Optional API functions */
#define INCLUDE_vTaskPrioritySet             1
#define INCLUDE_uxTaskPriorityGet            1
#define INCLUDE_vTaskDelete                  1
#define INCLUDE_vTaskCleanUpResources        1
#define INCLUDE_vTaskSuspend                 1
#define INCLUDE_vTaskDelayUntil              1
#define INCLUDE_vTaskDelay                   1


/* Cortex-M specific interrupt configuration */
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS 4 /* STM32G4 uses 4 priority bits */
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority" function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY      15

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values). */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

/* Interrupt priorities used by the kernel port layer itself. These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY \
    ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
    
/* configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero
See https://www.freertos.org/Documentation/02-Kernel/03-Supported-devices/04-Demos/ARM-Cortex/RTOS-Cortex-M3-M4#relevance-to-the-rtos-kernel */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )


#endif /* FREERTOS_CONFIG_H */