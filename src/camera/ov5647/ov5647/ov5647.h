/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_cam_sensor_types.h"

#define OV5647_SCCB_ADDR   0x36
#define OV5647_PID         0x5647
#define OV5647_SENSOR_NAME "OV5647"

/**
 * @brief Choose the MIPI table used on detect and set_format(NULL).
 *
 * CameraController calls this before ESPVideoClass::begin(). Width and height must
 * match one compiled table: 800x1280, 800x640, 800x800, 1920x1080, 1280x960.
 */
esp_err_t ov5647_select_mipi_size(uint16_t width, uint16_t height);

/**
 * @brief Power on camera sensor device and detect the device connected to the designated sccb bus.
 *
 * @param[in] config Configuration related to device power-on and detection.
 * @return
 *      - Camera device handle on success, otherwise, failed.
 */
esp_cam_sensor_device_t *ov5647_detect(esp_cam_sensor_config_t *config);

#ifdef __cplusplus
}
#endif
