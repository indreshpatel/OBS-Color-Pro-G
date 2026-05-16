// clang-format off #include <obs-module.h>
#include <graphics/vec4.h>
#include <graphics/vec3.h>
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

// 1. Fixed Unused Parameter error
const char *color_pro_get_name(void *unused) {
    UNUSED_PARAMETER(unused);
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

    // 2. Fixed vec3_from_rgba Error using standard OBS vec4 functions
    struct vec4 t_shadows, t_midtones, t_highlights;
    vec4_from_rgba(&t_shadows, c_shadows);
    vec4_from_rgba(&t_midtones, c_midtones);
    vec4_from_rgba(&t_highlights, c_highlights);

    vec3_set(&filter->balance_shadows, t_shadows.x, t_shadows.y, t_shadows.z);
    vec3_set(&filter->balance_midtones, t_midtones.x, t_midtones.y, t_midtones.z);
    vec3_set(&filter->balance_highlights, t_highlights.x, t_highlights.y, t_highlights.z);
}

// 3. Fixed unused effect parameter error
void color_pro_video_render(void *data, gs_effect_t *effect) {
    UNUSED_PARAMETER(effect); 
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

    // 4. Fixed gs_effect_render Error (obs_source_process_filter_end handles it)
    obs_source_process_filter_begin(filter->context, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING);
    obs_source_process_filter_end(filter->context, filter->effect, 0, 0);

    obs_leave_graphics();
}

// 5. Fixed unused data parameter error
obs_properties_t *color_pro_get_properties(void *data) {
    UNUSED_PARAMETER(data);
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

// 6. Fixed Windows C7555 Error using C++ Lambda Initialization
struct obs_source_info color_pro_filter_info = []() {
    struct obs_source_info info = {};
    info.id = "color_pro_filter";
    info.type = OBS_SOURCE_TYPE_FILTER;
    info.output_flags = OBS_SOURCE_VIDEO;
    info.get_name = color_pro_get_name;
    info.create = color_pro_create;
    info.destroy = color_pro_destroy;
    info.get_defaults = color_pro_get_defaults;
    info.get_properties = color_pro_get_properties;
    info.update = color_pro_update;
    info.video_render = color_pro_video_render;
    return info;
}();

// 7. MISSING OBS LOAD COMMANDS ADDED! (Warna plugin OBS mein load hi nahi hota)
// --- File ka aakhri hissa ---

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("Prince Studio")

bool obs_module_load(void) {
    obs_register_source(&color_pro_filter_info);
    return true;
}

void obs_module_unload(void) {
    // Plugin band hone par yahan cleanup hota hai
}

// ---------------------------
