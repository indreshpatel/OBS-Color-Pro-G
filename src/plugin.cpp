#include "plugin.h"
#include <util/platform.h>
#include <util/dstr.h>

#define SETTING_TEMP          "temperature"
#define SETTING_TINT          "tint"
#define SETTING_SAT           "saturation"
#define SETTING_EXPOSURE      "exposure"
#define SETTING_CONTRAST      "contrast"
#define SETTING_HIGHLIGHTS    "highlights"
#define SETTING_SHADOWS       "shadows"
#define SETTING_WHITES        "whites"
#define SETTING_BLACKS        "blacks"
#define SETTING_BAL_SHADOWS   "bal_shadows"
#define SETTING_BAL_MIDTONES  "bal_midtones"
#define SETTING_BAL_HIGHLIGHTS "bal_highlights"

const char *color_pro_get_name(void *unused) {
    return "Color Pro Filter";
}

void color_pro_get_defaults(obs_data_t *settings) {
    obs_data_set_default_double(settings, SETTING_TEMP, 0.0);
    obs_data_set_default_double(settings, SETTING_TINT, 0.0);
    obs_data_set_default_double(settings, SETTING_SAT, 1.0);
    obs_data_set_default_double(settings, SETTING_EXPOSURE, 0.0);
    obs_data_set_default_double(settings, SETTING_CONTRAST, 1.0);
    obs_data_set_default_double(settings, SETTING_HIGHLIGHTS, 0.0);
    obs_data_set_default_double(settings, SETTING_SHADOWS, 0.0);
    obs_data_set_default_double(settings, SETTING_WHITES, 0.0);
    obs_data_set_default_double(settings, SETTING_BLACKS, 0.0);
    
    obs_data_set_default_int(settings, SETTING_BAL_SHADOWS, 0xFFFFFF);
    obs_data_set_default_int(settings, SETTING_BAL_MIDTONES, 0xFFFFFF);
    obs_data_set_default_int(settings, SETTING_BAL_HIGHLIGHTS, 0xFFFFFF);
}

void *color_pro_create(obs_data_t *settings, obs_source_t *context) {
    struct color_pro_filter_data *filter = (struct color_pro_filter_data *)bzalloc(sizeof(struct color_pro_filter_data));
    filter->context = context;

    char *effect_path = obs_module_file("color_correction.effect");
    obs_enter_graphics();
    filter->effect = gs_effect_create_from_file(effect_path, NULL);
    if (filter->effect) {
        filter->param_temp_tint_sat = gs_effect_get_param_by_name(filter->effect, "temp_tint_sat");
        filter->param_exposure_contrast = gs_effect_get_param_by_name(filter->effect, "exposure_contrast");
        filter->param_tone_adjust = gs_effect_get_param_by_name(filter->effect, "tone_adjust");
        filter->param_bal_shadows = gs_effect_get_param_by_name(filter->effect, "bal_shadows");
        filter->param_bal_midtones = gs_effect_get_param_by_name(filter->effect, "bal_midtones");
        filter->param_bal_highlights = gs_effect_get_param_by_name(filter->effect, "bal_highlights");
    }
    obs_leave_graphics();
    bfree(effect_path);

    color_pro_update(filter, settings);
    return filter;
}

void color_pro_destroy(void *data) {
    struct color_pro_filter_data *filter = (struct color_pro_filter_data *)data;
    if (filter->effect) {
        obs_enter_graphics();
        gs_effect_destroy(filter->effect);
        obs_leave_graphics();
    }
    bfree(filter);
}

void color_pro_update(void *data, obs_data_t *settings) {
    struct color_pro_filter_data *filter = (struct color_pro_filter_data *)data;

    filter->temperature = (float)obs_data_get_double(settings, SETTING_TEMP);
    filter->tint = (float)obs_data_get_double(settings, SETTING_TINT);
    filter->saturation = (float)obs_data_get_double(settings, SETTING_SAT);
    filter->exposure = (float)obs_data_get_double(settings, SETTING_EXPOSURE);
    filter->contrast = (float)obs_data_get_double(settings, SETTING_CONTRAST);
    filter->highlights = (float)obs_data_get_double(settings, SETTING_HIGHLIGHTS);
    filter->shadows = (float)obs_data_get_double(settings, SETTING_SHADOWS);
    filter->whites = (float)obs_data_get_double(settings, SETTING_WHITES);
    filter->blacks = (float)obs_data_get_double(settings, SETTING_BLACKS);

    uint32_t c_shadows = (uint32_t)obs_data_get_int(settings, SETTING_BAL_SHADOWS);
    uint32_t c_midtones = (uint32_t)obs_data_get_int(settings, SETTING_BAL_MIDTONES);
    uint32_t c_highlights = (uint32_t)obs_data_get_int(settings, SETTING_BAL_HIGHLIGHTS);

    vec3_from_rgba(&filter->balance_shadows, c_shadows);
    vec3_from_rgba(&filter->balance_midtones, c_midtones);
    vec3_from_rgba(&filter->balance_highlights, c_highlights);
}

void color_pro_video_render(void *data, gs_effect_t *effect) {
    struct color_pro_filter_data *filter = (struct color_pro_filter_data *)data;
    if (!filter->effect) {
        obs_source_skip_video_filter(filter->context);
        return;
    }

    obs_enter_graphics();

    struct vec3 v_temp_tint_sat;
    vec3_set(&v_temp_tint_sat, filter->temperature, filter->tint, filter->saturation);
    gs_effect_set_vec3(filter->param_temp_tint_sat, &v_temp_tint_sat);

    struct vec2 v_exp_con;
    vec2_set(&v_exp_con, filter->exposure, filter->contrast);
    gs_effect_set_vec2(filter->param_exposure_contrast, &v_exp_con);

    struct vec4 v_tone;
    vec4_set(&v_tone, filter->highlights, filter->shadows, filter->whites, filter->blacks);
    gs_effect_set_vec4(filter->param_tone_adjust, &v_tone);

    gs_effect_set_vec3(filter->param_bal_shadows, &filter->balance_shadows);
    gs_effect_set_vec3(filter->param_bal_midtones, &filter->balance_midtones);
    gs_effect_set_vec3(filter->param_bal_highlights, &filter->balance_highlights);

    obs_source_process_filter_begin(filter->context, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING);
    gs_effect_render(filter->effect, "Draw");
    obs_source_process_filter_end(filter->context, filter->effect, 0, 0);

    obs_leave_graphics();
}

obs_properties_t *color_pro_get_properties(void *data) {
    obs_properties_t *props = obs_properties_create();

    // Color Adjustments Group
    obs_properties_t *color_group = obs_properties_create();
    obs_properties_add_float_slider(color_group, SETTING_TEMP, "Temperature", -1.0, 1.0, 0.01);
    obs_properties_add_float_slider(color_group, SETTING_TINT, "Tint", -1.0, 1.0, 0.01);
    obs_properties_add_float_slider(color_group, SETTING_SAT, "Saturation", 0.0, 2.0, 0.01);
    obs_properties_add_group(props, "color_adj", "Color Adjustments", OBS_GROUP_NORMAL, color_group);

    // Light/Tone Adjustments Group
    obs_properties_t *light_group = obs_properties_create();
    obs_properties_add_float_slider(light_group, SETTING_EXPOSURE, "Exposure", -5.0, 5.0, 0.1);
    obs_properties_add_float_slider(light_group, SETTING_CONTRAST, "Contrast", 0.0, 2.0, 0.01);
    obs_properties_add_float_slider(light_group, SETTING_HIGHLIGHTS, "Highlights", -1.0, 1.0, 0.01);
    obs_properties_add_float_slider(light_group, SETTING_SHADOWS, "Shadows", -1.0, 1.0, 0.01);
    obs_properties_add_float_slider(light_group, SETTING_WHITES, "Whites", -1.0, 1.0, 0.01);
    obs_properties_add_float_slider(light_group, SETTING_BLACKS, "Blacks", -1.0, 1.0, 0.01);
    obs_properties_add_group(props, "light_adj", "Light/Tone Adjustments", OBS_GROUP_NORMAL, light_group);

    // Color Balance Group
    obs_properties_t *balance_group = obs_properties_create();
    obs_properties_add_color(balance_group, SETTING_BAL_SHADOWS, "Shadow Balance");
    obs_properties_add_color(balance_group, SETTING_BAL_MIDTONES, "Midtone Balance");
    obs_properties_add_color(balance_group, SETTING_BAL_HIGHLIGHTS, "Highlight Balance");
    obs_properties_add_group(props, "color_bal", "Color Balance", OBS_GROUP_NORMAL, balance_group);

    return props;
}

struct obs_source_info color_pro_filter_info = {
    .id = "color_pro_filter",
    .type = OBS_SOURCE_TYPE_FILTER,
    .output_flags = OBS_SOURCE_VIDEO,
    .get_name = color_pro_get_name,
    .create = color_pro_create,
    .destroy = color_pro_destroy,
    .update = color_pro_update,
    .get_properties = color_pro_get_properties,
    .get_defaults = color_pro_get_defaults,
    .video_render = color_pro_video_render,
};
