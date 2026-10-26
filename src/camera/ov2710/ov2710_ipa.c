/*
* SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
*
* SPDX-License-Identifier: ESPRESSIF MIT
*/

#include "../../../user_setup.h"

#if defined(CAM_OV2710)

#include <string.h>
#include "esp_ipa.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

typedef struct esp_video_ipa_index {
    const char *name;
    const char *description;
    const esp_ipa_config_t *ipa_config;
} esp_video_ipa_index_t;

static const esp_ipa_awb_config_t s_ipa_awb_OV2710_0_config = {
    .model = ESP_IPA_AWB_MODEL_0,
    .min_counted = 2000,
    .min_red_gain_step = 0.34,
    .min_blue_gain_step = 0.4,
    .red_gain_scale = 1.000000f,
    .blue_gain_scale = 1.000000f,
    .range = {
        .green_max = 190,
        .green_min = 81,
        .rg_max = 0.9096,
        .rg_min = 0.573,
        .bg_max = 0.9634,
        .bg_min = 0.5368
    },
    .green_luma_env = "dummy_awb_luma",
    .green_luma_init = 91,
    .green_luma_step_ratio = 0.3,
    .enable_sub_win = false,
    .min_subwin_wp_counted = 0,
    .min_subwin_participated = 0,
    .subwin_weight = {
        { 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f },
        { 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f },
        { 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f },
        { 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f },
        { 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f },
    },
    .subwin_green_dark = 0,
    .subwin_green_mid = 100,
    .subwin_green_bright = 200,
    .zones = NULL,
    .zones_count = 0,
    .ref_points = NULL,
    .ref_points_count = 0,
    .new_w = 0.300000f,
    .prev_w = 0.700000f,
    .export_ct = false,
    .outlier_rg = 0.000000f,
    .outlier_bg = 0.000000f,
    .zone_hysteresis_ratio = 0.000000f,
    .zone_switch_count = 0,
    .type_counter_max = 20000,
};

static const float s_ipa_ian_env_speed_OV2710_0[] = {
    -0.005463f, -0.010018f, 0.000000f, 0.033241f, 0.085583f, 0.136704f, 0.160734f, 0.148777f,
    0.148777f, 0.160734f, 0.136704f, 0.085583f, 0.033241f, 0.000000f, -0.010018f, -0.005463f,
};

static const esp_ipa_ian_luma_ae_config_t s_esp_ipa_ian_luma_ae_OV2710_0_config = {
    .weight = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    },
};

static const esp_ipa_ian_luma_env_config_t s_esp_ipa_ian_luma_env_OV2710_0_config = {
    .k = 250000,
    .speed_param = s_ipa_ian_env_speed_OV2710_0,
    .speed_param_size = ARRAY_SIZE(s_ipa_ian_env_speed_OV2710_0),
    .weight = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    },
};

static const esp_ipa_ian_luma_config_t s_esp_ipa_ian_luma_OV2710_0_config = {
    .ae = &s_esp_ipa_ian_luma_ae_OV2710_0_config,
    .env = &s_esp_ipa_ian_luma_env_OV2710_0_config,
};

static const esp_ipa_atc_luma_lut_t s_ipa_atc_luma_lut_OV2710_0[] = {
    { .luma = 250, .ae_value = 48 },
    { .luma = 500, .ae_value = 60 },
    { .luma = 750, .ae_value = 75 },
};

static const esp_ipa_atc_config_t s_ipa_atc_OV2710_0_config = {
    .model = ESP_IPA_ATC_MODEL_0,
    .init_value = 48,
    .delay_frames = 3,
    .luma_env = "env.luma.avg",
    .min_ae_value_step = 5,
    .luma_lut = s_ipa_atc_luma_lut_OV2710_0,
    .luma_lut_size = ARRAY_SIZE(s_ipa_atc_luma_lut_OV2710_0),
};

static const esp_ipa_ian_config_t s_ipa_ian_OV2710_0_config = {
    .luma = &s_esp_ipa_ian_luma_OV2710_0_config,
};

static const esp_ipa_acc_sat_t s_ipa_acc_sat_OV2710_0_config[] = {
    {
        .color_temp = 0,
        .saturation = 128
    },
};

static const esp_ipa_acc_ccm_unit_t s_esp_ipa_acc_ccm_OV2710_0_table[] = {
    {
        .color_temp = 4310,
        .ccm = {
            .matrix = {
                { 2.0606f, -0.7608f, -0.2998f },
                { -0.3150f, 1.4163f, -0.1012f },
                { -0.3664f, -0.9836f, 2.3500f }
            }
        }
    },
};

static const esp_ipa_acc_ccm_config_t s_esp_ipa_acc_ccm_OV2710_0_config = {
    .model = 0,
    .luma_env = "ae.luma.avg",
    .luma_low_threshold = 28,
    .luma_low_ccm = {
        .matrix = {
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f }
        }
    },
    .ccm_table = s_esp_ipa_acc_ccm_OV2710_0_table,
    .ccm_table_size = 1,
};

static const esp_ipa_acc_config_t s_ipa_acc_OV2710_0_config = {
    .sat_table = s_ipa_acc_sat_OV2710_0_config,
    .sat_table_size = ARRAY_SIZE(s_ipa_acc_sat_OV2710_0_config),
    .ccm = &s_esp_ipa_acc_ccm_OV2710_0_config,
};

static const esp_ipa_adn_bf_t s_ipa_adn_bf_OV2710_0_config[] = {
    {
        .gain = 1,
        .bf = {
            .level = 3,
            .matrix = {
                {1, 3, 1},
                {3, 5, 3},
                {1, 3, 1}
            }
        }
    },
};

static const esp_ipa_adn_config_t s_ipa_adn_OV2710_0_config = {
    .bf_table = s_ipa_adn_bf_OV2710_0_config,
    .bf_table_size = ARRAY_SIZE(s_ipa_adn_bf_OV2710_0_config),
    .dm_table = NULL,
    .dm_table_size = 0,
};

static const esp_ipa_aen_gamma_unit_t s_esp_ipa_aen_gamma_OV2710_0_table[] = {
    {
        .luma = 75.1,
        .gamma = {
            .red = {
                .x = { 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255,  },
                .y = { 63, 90, 111, 129, 144, 159, 172, 184, 196, 207, 217, 227, 237, 246, 255, 255,  }
            },
            .green = {
                .x = { 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255,  },
                .y = { 63, 90, 111, 129, 144, 159, 172, 184, 196, 207, 217, 227, 237, 246, 255, 255,  }
            },
            .blue = {
                .x = { 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255,  },
                .y = { 63, 90, 111, 129, 144, 159, 172, 184, 196, 207, 217, 227, 237, 246, 255, 255,  }
            }
        }
    },
};

static const esp_ipa_aen_gamma_config_t s_ipa_aen_gamma_OV2710_0_config = {
    .model = 0,
    .luma_env = "ae.luma.avg",
    .luma_min_step = 16.0,
    .gamma_table = s_esp_ipa_aen_gamma_OV2710_0_table,
    .gamma_table_size = 1,
};

static const esp_ipa_aen_sharpen_t s_ipa_aen_sharpen_OV2710_0_config[] = {
    {
        .gain = 1,
        .sharpen = {
            .h_thresh = 16,
            .l_thresh = 3,
            .h_coeff = 2.65f,
            .m_coeff = 2.95f,
            .matrix = {
                {1, 2, 1},
                {2, 2, 2},
                {1, 2, 1}
            }
        }
    },
};

static const esp_ipa_aen_con_t s_ipa_aen_con_OV2710_0_config[] = {
    {
        .gain = 1,
        .contrast = 130
    },
};

static const esp_ipa_aen_config_t s_ipa_aen_OV2710_0_config = {
    .gamma = &s_ipa_aen_gamma_OV2710_0_config,
    .sharpen_table = s_ipa_aen_sharpen_OV2710_0_config,
    .sharpen_table_size = ARRAY_SIZE(s_ipa_aen_sharpen_OV2710_0_config),
    .con_table = s_ipa_aen_con_OV2710_0_config,
    .con_table_size = ARRAY_SIZE(s_ipa_aen_con_OV2710_0_config),
};

static const char *s_ipa_OV2710_0_names[] = {
    "esp_ipa_awb",
    "esp_ipa_ian",
    "esp_ipa_acc",
    "esp_ipa_adn",
    "esp_ipa_aen",
    "esp_ipa_atc",
};

static const esp_ipa_config_t s_ipa_OV2710_0_config = {
    .names = s_ipa_OV2710_0_names,
    .nums = ARRAY_SIZE(s_ipa_OV2710_0_names),
    .version = 1,
    .awb = &s_ipa_awb_OV2710_0_config,
    .ian = &s_ipa_ian_OV2710_0_config,
    .acc = &s_ipa_acc_OV2710_0_config,
    .adn = &s_ipa_adn_OV2710_0_config,
    .aen = &s_ipa_aen_OV2710_0_config,
    .atc = &s_ipa_atc_OV2710_0_config,
};

static const esp_video_ipa_index_t s_video_ipa_configs[] = {
    {
        .name = "OV2710",
        .description = "0",
        .ipa_config = &s_ipa_OV2710_0_config
    },
};

const esp_ipa_config_t *esp_ipa_pipeline_get_config(const char *name)
{
    for (int i = 0; i < ARRAY_SIZE(s_video_ipa_configs); i++) {
        if (!strcmp(name, s_video_ipa_configs[i].name)) {
            return s_video_ipa_configs[i].ipa_config;
        }
    }
    return NULL;
}

const esp_ipa_config_t *esp_ipa_pipeline_enum_configs(const char *sensor_name, int index)
{
    int n = 0;
    if (!sensor_name || index < 0) {
        return NULL;
    }
    for (int i = 0; i < ARRAY_SIZE(s_video_ipa_configs); i++) {
        if (!strcmp(sensor_name, s_video_ipa_configs[i].name)) {
            if (n == index) {
                return s_video_ipa_configs[i].ipa_config;
            }
            n++;
        }
    }
    return NULL;
}

void displaywx_link_ov2710_ipa(void)
{
}

#endif /* CAM_OV2710 */

/* Json oficial ov2710_default.json: sem AGC (o sensor so expoe AE_LEVEL).
 * AWB do template Arduino. ATC + IAN env do JSON. IAN color_temp / gamma_param omitidos. */
