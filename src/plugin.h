#pragma once

#include <obs-module.h>
#include <graphics/vec3.h>
#include <graphics/vec4.h>

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

    // Color Balance (Shadows, Midtones, Highlights)
    struct vec3 balance_shadows;
    struct vec3 balance_midtones;
    struct vec3 balance_highlights;

    // Curves (Simplified as control points for this scaffold)
    // In a real implementation, these would be lookup tables (LUTs) or spline coefficients.
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

// OBS Source Info callbacks
extern const char *color_pro_get_name(void *unused);
extern void *color_pro_create(obs_data_t *settings, obs_source_t *context);
extern void color_pro_destroy(void *data);
extern void color_pro_update(void *data, obs_data_t *settings);
extern void color_pro_video_render(void *data, gs_effect_t *effect);
extern obs_properties_t *color_pro_get_properties(void *data);
extern void color_pro_get_defaults(obs_data_t *settings);
