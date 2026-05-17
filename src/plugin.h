/*
Plugin Name: OBS Color Pro
Developer: Prince Studio
*/

#pragma once

// C++ ko batane ke liye ki yeh OBS (C language) ke rules hain
#ifdef __cplusplus
extern "C" {
#endif

// Original OBS Headers
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

// Plugin Specific Headers
#include <obs-module.h>
#include <graphics/vec3.h>
#include <graphics/vec4.h>

// Original plugin-support connections
extern const char *PLUGIN_NAME;
extern const char *PLUGIN_VERSION;
void obs_log(int log_level, const char *format, ...);
extern void blogva(int log_level, const char *format, va_list args);

/**
 * @brief Data structure for the Color Pro filter instance.
 */
struct color_pro_filter_data {
    obs_source_t *context;
    gs_effect_t *effect;

    // Color Adjustments
    float temperature;
    float tint;
    float saturation;

    // Light/Tone Adjustments
    float exposure;
    float contrast;
    float highlights;
    float shadows;
    float whites;
    float blacks;

    // Color Balance
    struct vec3 balance_shadows;
    struct vec3 balance_midtones;
    struct vec3 balance_highlights;

    // Curves
    struct vec4 curve_master;
    struct vec4 curve_red;
    struct vec4 curve_green;
    struct vec4 curve_blue;

    // Shader parameter handles
    gs_eparam_t *param_temp_tint_sat;
    gs_eparam_t *param_exposure_contrast;
    gs_eparam_t *param_tone_adjust;
    gs_eparam_t *param_bal_shadows;
    gs_eparam_t *param_bal_midtones;
    gs_eparam_t *param_bal_highlights;
};

// OBS Source Info callbacks (Ab inke naam C++ nahi badlega)
extern const char *color_pro_get_name(void *unused);
extern void *color_pro_create(obs_data_t *settings, obs_source_t *context);
extern void color_pro_destroy(void *data);
extern void color_pro_update(void *data, obs_data_t *settings);
extern void color_pro_video_render(void *data, gs_effect_t *effect);
extern obs_properties_t *color_pro_get_properties(void *data);
extern void color_pro_get_defaults(obs_data_t *settings);

#ifdef __cplusplus
}
#endif
