/**
 * @file cattle_ai_tracker_app.h
 */

 #ifndef CATTLE_AI_TRACKER_APP_H
 #define CATTLE_AI_TRACKER_APP_H

 #ifdef __cplusplus
 extern "C" {
 #endif

#include "../lvgl/lvgl.h"

/* Font declarations */
/* FontAwesome fonts */
LV_FONT_DECLARE(font_awesome_14_1);
LV_FONT_DECLARE(font_awesome_16_4);
LV_FONT_DECLARE(font_awesome_20_4);
LV_FONT_DECLARE(font_awesome_30_1);
LV_FONT_DECLARE(font_awesome_30_4);

/* PuHui fonts */
LV_FONT_DECLARE(font_puhui_14_1);
LV_FONT_DECLARE(font_puhui_16_2);
LV_FONT_DECLARE(font_puhui_16_4);
LV_FONT_DECLARE(font_puhui_18_2);
LV_FONT_DECLARE(font_puhui_20_2);
LV_FONT_DECLARE(font_puhui_20_4);
// LV_FONT_DECLARE(font_puhui_30_4);

/* Target marker colors */
 typedef enum {
     TARGET_COLOR_GREEN = 0x7ED643,  /* 绿 - Green */
     TARGET_COLOR_YELLOW = 0xFFA000, /* 黄 - Yellow */
     TARGET_COLOR_PINK = 0xEC8FD4,   /* 粉 - Pink */
     TARGET_COLOR_CYAN = 0x78CFD1,   /* 青 - Cyan */
     TARGET_COLOR_PURPLE = 0x8A66F9, /* 紫 - Purple */
     TARGET_COLOR_COW = 0x6A6AF2     /* 牛 - Cow (special marker) */
 } target_color_t;

 #ifndef CATTLE_SCREEN_WIDTH
 #define CATTLE_SCREEN_WIDTH 466
 #endif
 #ifndef CATTLE_SCREEN_HEIGHT
 #define CATTLE_SCREEN_HEIGHT 466
 #endif

 void lv_demo_cattle_ai_tracker(void);

/* Bottom text display function
 * Manual line breaking optimized for circular screen geometry
 *
 * Display Capacity (varies per line due to circular boundary):
 *   - Line 1 (top):    280px width (~14-15 Chinese chars at 18px font)
 *   - Line 2 (middle): 240px width (~12-13 Chinese chars)
 *   - Line 3 (bottom): 180px width (~9-10 Chinese chars)
 *   - Total capacity: ~35-38 characters across 3 lines
 *
 * Circular Geometry:
 *   - Dynamic vertical positioning based on line count:
 *     * 1 line:  y=-50 (positioned higher to stay within boundary)
 *     * 2 lines: y=-35 (middle position)
 *     * 3 lines: y=-20 (near bottom edge)
 *   - Each line has different width matching circular boundary
 *   - Top line wider, bottom line narrower (pyramid shape)
 *   - All text stays within circular border
 *
 * Implementation:
 *   - Manual line breaking using lv_txt_get_width() for precise measurement
 *   - No automatic LVGL wrapping - full control over each line
 *   - Inserts \n at optimal break points for each line's width
 *   - UTF-8 aware (Chinese characters supported)
 *   - Adds "..." ellipsis when text exceeds capacity
 *   - Auto-adjusts vertical position to prevent boundary overflow
 *
 * Example usage:
 *   update_idle_bottom_text("状态正常");                     // 1 line (y=-50)
 *   update_idle_bottom_text("系统正在处理数据请稍候等待");    // 2 lines (y=-35)
 *   update_idle_bottom_text("完成核心硬件开发解决基础问题"); // 3 lines (y=-20)
 *   update_idle_bottom_text("超长文本..."); // Truncated with "..."
 */
void update_idle_bottom_text(const char *text);

 /* GPS API Functions */
 void gps_add_target(float lat, float lon, uint32_t color);
 void gps_add_target_at_distance(float distance_meters, float bearing_degrees, uint32_t color);
 void gps_remove_target(int index);
 void gps_clear_all_targets(void);
 void gps_set_tracker_position(float lat, float lon);
 int gps_get_target_count(void);
 int gps_get_target_distance(int index);
 void gps_update_target_markers(void);
 void gps_mark_data_dirty(void);

 /* Dummy data access functions */
 float gps_get_dummy_self_lat(void);
 float gps_get_dummy_self_lon(void);
 int gps_get_dummy_target_count(void);
 const void *gps_get_dummy_target(int index);

 #ifdef __cplusplus
 }
 #endif

 #endif /* CATTLE_AI_TRACKER_APP_H */
