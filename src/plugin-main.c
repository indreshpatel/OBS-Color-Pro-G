/*
Plugin Name: OBS Color Pro
Developer: Prince Studio
*/

#include <obs-module.h>
#include <plugin-support.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

// C++ wali file se connection lene ke liye
extern void register_color_pro_filter(void);

bool obs_module_load(void)
{
    obs_log(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
    
    // Yahan humne OBS ko apne Color Pro Filter ke baare mein bata diya!
    register_color_pro_filter();
    
    return true;
}

void obs_module_unload(void)
{
    obs_log(LOG_INFO, "plugin unloaded");
}
// (Yahan bhi aakhri line par ek Enter/Khali jagah zaroor rakhein)
