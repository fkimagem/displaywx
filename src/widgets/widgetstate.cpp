#include "widgetstate.h"

SemaphoreHandle_t dwx_widget_mutex()
{
    static StaticSemaphore_t storage;
    static SemaphoreHandle_t mutex = xSemaphoreCreateRecursiveMutexStatic(&storage);
    return mutex;
}
