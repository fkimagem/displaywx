/*
 * Arduino-ESP32 3.3.11 builds esp_cam_sensor with STATIC_STORE detection.
 * This object replaces esp_cam_sensor_detect_get_array() and probes SC2336 only.
 */

#include "../../../user_setup.h"

#if defined(CAM_SC2336)

#include "esp_log.h"
#include "esp_cam_sensor_detect.h"
#include "sc2336/sc2336.h"

static const char *TAG = "sc2336_hook";

static esp_cam_sensor_device_t *sc2336_detect_mipi(void *config)
{
    ((esp_cam_sensor_config_t *)config)->sensor_port = ESP_CAM_SENSOR_MIPI_CSI;
    return sc2336_detect(config);
}

static const esp_cam_sensor_detect_fn_t s_detect_fns[] = {
    {
        .detect = sc2336_detect_mipi,
        .port = ESP_CAM_SENSOR_MIPI_CSI,
        .sccb_addr = SC2336_SCCB_ADDR,
    },
};

void esp_cam_sensor_detect_get_array(esp_cam_sensor_detect_fn_t **array_start_ptr, esp_cam_sensor_detect_fn_t **array_end_ptr)
{
    ESP_LOGI(TAG, "using SC2336 detect table, SCCB addr=0x%x", SC2336_SCCB_ADDR);
    *array_start_ptr = (esp_cam_sensor_detect_fn_t *)s_detect_fns;
    *array_end_ptr = (esp_cam_sensor_detect_fn_t *)(s_detect_fns + 1);
}

void displaywx_link_sc2336_hook(void)
{
}

#endif /* CAM_SC2336 */
