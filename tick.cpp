#include "FreeRTOS.h"
#include "task.h"

// function that's safe to call from threads as well as ISRs
TickType_t GetTickCount()
{
    if(xPortIsInsideInterrupt())
    {
        return xTaskGetTickCountFromISR();
    }
    return xTaskGetTickCount();
}

