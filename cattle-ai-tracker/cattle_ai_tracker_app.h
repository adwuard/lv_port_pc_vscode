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
LV_FONT_DECLARE(montserrat_time_82_extra_bold);


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

/* Bottom text display function with typewriter animation
 * Displays text with character-by-character typewriter effect and sliding window
 *
 * Display Capacity (varies per line due to circular boundary):
 *   - Line 1 (top):    280px width (~14-15 Chinese chars at 18px font)
 *   - Line 2 (middle): 240px width (~12-13 Chinese chars)
 *   - Line 3 (bottom): 180px width (~9-10 Chinese chars)
 *   - Display window: ~38 characters maximum (across 3 lines)
 *
 * Animation Features:
 *   - Typewriter effect: Characters appear one by one (50ms/char, ~20 chars/sec)
 *   - Sliding window: For long text, window slides through entire text
 *   - Adaptive speed:
 *     * Fast typewriter: 50ms per character (initial reveal)
 *     * Slow scroll: 150ms per character (when window is full and scrolling)
 *     * Punctuation pause: 300ms after Chinese comma "，" or period "。"
 *   - Natural reading rhythm with pauses at sentence boundaries
 *   - Smooth character-by-character reveal
 *   - Auto-loop: After completing, pauses 1 second, clears, and restarts
 *   - UTF-8 aware animation (handles Chinese characters correctly)
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
 *   - Dynamic timer period (50ms for typewriter, 150ms for scrolling)
 *   - Sliding window for text longer than display capacity
 *   - Auto-loops continuously for long text
 *   - Auto-cancels previous animation when new text is set
 *   - Memory managed automatically (strdup/free)
 *
 * Animation Flow (for long text):
 *   1. Fast typewriter reveal (first 38 chars)
 *   2. Slow scroll through remaining text
 *   3. Pause at end (~1 second)
 *   4. Clear display
 *   5. Loop back to step 1
 *
 * Example usage:
 *   update_idle_bottom_text("状态正常");  // Short text - typewriter only
 *   update_idle_bottom_text("很长的文本会先快速显示再慢速滚动然后循环"); // Long text - full animation cycle
 *   update_idle_bottom_text(NULL);       // Clear text and stop animation
 */
void update_idle_bottom_text(const char *text);

/* Eye animation control with dynamic behaviors
 *
 * Animation Features:
 *   - Random blinking: Blinks occur at random intervals (3-7 seconds)
 *   - Variable blink speed: Ultra-fast blinks (100-160ms total duration)
 *   - Eye movement: Pupils look in random directions (left, right, up, down, diagonals, center)
 *   - Smooth transitions: Pupils move gradually to target positions (2px per step)
 *   - Continuous loop: Animations run indefinitely with natural timing
 *   - Emotion rendering: Partial state updates for different emotions
 *
 * Eye States:
 *   0 = idle (normal with automatic random blinking and looking around)
 *   1 = blinking (quick oval blink, manual trigger or automatic)
 *   2 = happy (squinted eyes, returns to idle after ~500ms)
 *
 * Movement Patterns:
 *   - Look left/right: ±18px horizontal offset
 *   - Look up/down: ±18px vertical offset
 *   - Look diagonals: ±9px both axes
 *   - Return to center: 0,0 offset
 *   - Hold duration: ~1 second per position
 *   - Movement speed: 2px per 50ms (smooth and fast)
 *
 * Keyboard Controls:
 *   N - Reset to Normal/Idle state (automatic animations)
 *   H - Happy eyes (squinted expression)
 *   B - Manual Blink (trigger quick blink)
 *   L - Look Left
 *   R - Look Right
 *   U - Look Up
 *   D - Look Down
 *   C - Look Center
 *   E - Random Eye movement (picks random direction)
 *
 * Example usage:
 *   set_idle_eye_state(0);  // Normal idle with random blinking and eye movements
 *   set_idle_eye_state(2);  // Happy eyes (or press 'H' key)
 *   set_idle_eye_state(1);  // Trigger blink (or press 'B' key)
 */
void set_idle_eye_state(int state);

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
