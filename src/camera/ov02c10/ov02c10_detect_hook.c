/*
 * Arduino-ESP32 3.3.11 builds esp_cam_sensor with STATIC_STORE detection.
 * OV02C10 is absent from that prebuilt table, so this object replaces
 * esp_cam_sensor_detect_get_array() and probes OV02C10 first.
 */

#include "../../../user_setup.h"

#if defined(CAM_OV02C10)

#include "esp_log.h"
#include "esp_cam_sensor_detect.h"
#include "ov02c10/ov02c10.h"

static const char *TAG = "ov02c10_hook";

static esp_cam_sensor_device_t *ov02c10_detect_mipi(void *config)
{
    ((esp_cam_sensor_config_t *)config)->sensor_port = ESP_CAM_SENSOR_MIPI_CSI;
    return ov02c10_detect(config);
}

static const esp_cam_sensor_detect_fn_t s_detect_fns[] = {
    {
        .detect = ov02c10_detect_mipi,
        .port = ESP_CAM_SENSOR_MIPI_CSI,
        .sccb_addr = OV02C10_SCCB_ADDR,
    },
};

void esp_cam_sensor_detect_get_array(esp_cam_sensor_detect_fn_t **array_start_ptr, esp_cam_sensor_detect_fn_t **array_end_ptr)
{
    ESP_LOGI(TAG, "using OV02C10 detect table, SCCB addr=0x%x", OV02C10_SCCB_ADDR);
    *array_start_ptr = (esp_cam_sensor_detect_fn_t *)s_detect_fns;
    *array_end_ptr = (esp_cam_sensor_detect_fn_t *)(s_detect_fns + 1);
}

void displaywx_link_ov02c10_hook(void)
{
}

#endif /* CAM_OV02C10 */
