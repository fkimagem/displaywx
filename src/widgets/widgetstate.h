#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// One recursive task mutex serializes widget state and backend drawing.
// It has static storage and outlives DisplayWX and all application widgets.
SemaphoreHandle_t dwx_widget_mutex();

class WidgetStateGuard {
public:
    explicit WidgetStateGuard(TickType_t wait = portMAX_DELAY)
        : m_locked(xSemaphoreTakeRecursive(dwx_widget_mutex(), wait) == pdTRUE) {}
    ~WidgetStateGuard() { if (m_locked) xSemaphoreGiveRecursive(dwx_widget_mutex()); }
    explicit operator bool() const { return m_locked; }
    WidgetStateGuard(const WidgetStateGuard&) = delete;
    WidgetStateGuard& operator=(const WidgetStateGuard&) = delete;
private:
    bool m_locked;
};
