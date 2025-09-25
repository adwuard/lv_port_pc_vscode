/**
 * @file cattle_ai_tracker_app.c
 * Minimal, hardware-agnostic tracker UI demo for a 466x466 circular OLED.
 */

/*********************
 *      INCLUDES
 *********************/
#include "cattle_ai_tracker_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*********************
 *      DEFINES
 *********************/
#define CIRCLE_RADIUS            (CATTLE_SCREEN_WIDTH/2)
#define CIRCLE_CENTER            (CIRCLE_RADIUS)
#define SOS_HOLD_MS              3000
#define SETTINGS_PANEL_HEIGHT    (CATTLE_SCREEN_HEIGHT * 8 / 10)  // 80% of screen height

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_obj_t *screen;
    lv_obj_t *viewport;      /* circular clipped container */

    /* Screens */
    lv_obj_t *idle_screen;
    lv_obj_t *tracking_screen;
    lv_obj_t *sos_screen;

    /* Common overlays */
    lv_obj_t *settings_panel;
    lv_obj_t *settings_backdrop;
    lv_obj_t *settings_drag_tab;

    /* Tracking UI */
    lv_obj_t *compass_container;
    lv_obj_t *compass_dial;
    lv_obj_t *needle;
    lv_obj_t *needle_circle;
    lv_obj_t *compass_labels;
    lv_obj_t *calib_panel;
    bool      calibrated;
    float     yaw_deg;

    /* Dummy GPS/target */
    int       gps_sat_count;
    float     self_lat, self_lon;
    float     target_lat, target_lon;

    /* SOS */
    lv_obj_t *sos_hold_ring;
    lv_obj_t *sos_cancel_btn;
    lv_timer_t *sos_timer;
    uint32_t sos_pressed_start_ms;
    bool sos_active;

    /* Timers */
    lv_timer_t *tick_timer;

    /* Focus object for keyboard events */
    lv_obj_t *focus_obj;

    /* Idle screen bottom text */
    lv_obj_t *idle_bottom_text;
} cattle_app_t;

static cattle_app_t g;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void create_root(void);
static void create_idle_screen(void);
static void create_tracking_screen(void);
static void create_settings_panel(void);
static void create_sos_screen(void);

static void show_idle(void);
static void show_tracking(void);
static void show_sos_alert(void);
static void hide_sos_alert(void);

static void on_keyboard(lv_event_t *e);
static void on_pressed(lv_event_t *e);
static void on_pressing(lv_event_t *e);
static void on_released(lv_event_t *e);
static void on_gesture(lv_event_t *e);
static void on_settings_drag(lv_event_t *e);
static void on_settings_backdrop_click(lv_event_t *e);
static void on_sos_cancel(lv_event_t *e);
static void on_tick(lv_timer_t *t);
static void on_settings_close_anim_ready(lv_anim_t * anim);

static void compass_update(float yaw_deg);
static void compass_build(lv_obj_t *parent);
/* Calibration function removed */

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_demo_cattle_ai_tracker(void)
{
    memset(&g, 0, sizeof(g));

    /* Dummy data */
    g.gps_sat_count = 7;
    g.self_lat = 22.280f; g.self_lon = 114.158f; /* HK */
    g.target_lat = 22.284f; g.target_lon = 114.170f;
    g.calibrated = true;  // Start as calibrated to avoid showing calibration panel
    g.yaw_deg = 0.f;

    create_root();
    create_idle_screen();
    create_tracking_screen();
    create_settings_panel();
    create_sos_screen();

    show_idle();

    /* Create hidden focusable object for keyboard events */
    lv_obj_t *focus = lv_obj_create(g.screen);
    lv_obj_set_size(focus, 1, 1);
    lv_obj_add_flag(focus, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(focus, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_add_event_cb(focus, on_keyboard, LV_EVENT_KEY, NULL);
    lv_group_add_obj(lv_group_get_default(), focus);
    lv_group_focus_obj(focus);

    /* Store focus object globally for later use */
    g.focus_obj = focus;

    /* Global press handling */
    lv_obj_add_event_cb(g.screen, on_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(g.screen, on_pressing, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(g.screen, on_released, LV_EVENT_RELEASED, NULL);

    /* Gesture handling for settings */
    lv_obj_add_event_cb(g.screen, on_gesture, LV_EVENT_GESTURE, NULL);

    /* Drive dummy updates */
    g.tick_timer = lv_timer_create(on_tick, 100, NULL);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void create_root(void)
{
    g.screen = lv_obj_create(NULL);
    lv_obj_set_size(g.screen, CATTLE_SCREEN_WIDTH, CATTLE_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(g.screen, lv_color_hex(0x101214), 0);
    lv_obj_set_style_bg_opa(g.screen, LV_OPA_COVER, 0);

    /* Circular viewport centered - no border */
    g.viewport = lv_obj_create(g.screen);
    lv_obj_set_size(g.viewport, CATTLE_SCREEN_WIDTH, CATTLE_SCREEN_HEIGHT);
    lv_obj_center(g.viewport);
    lv_obj_set_style_radius(g.viewport, CIRCLE_RADIUS, 0);
    lv_obj_set_style_clip_corner(g.viewport, true, 0);
    lv_obj_set_style_border_width(g.viewport, 0, 0);
    lv_obj_set_style_bg_color(g.viewport, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(g.viewport, LV_OPA_COVER, 0);

    lv_screen_load(g.screen);
}

static void create_idle_screen(void)
{
    g.idle_screen = lv_obj_create(g.screen);
    lv_obj_remove_style_all(g.idle_screen);
    lv_obj_set_size(g.idle_screen, CATTLE_SCREEN_WIDTH, CATTLE_SCREEN_HEIGHT);
    lv_obj_set_style_bg_opa(g.idle_screen, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(g.idle_screen, LV_OBJ_FLAG_SCROLLABLE);

    /* Center label */
    lv_obj_t *label = lv_label_create(g.idle_screen);
    lv_label_set_text(label, "Cattle AI Tracker\nIdle\n\nPress 'T' for Tracking\nPress 'S' for Settings\nPress 'I' for Info");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(label);


    /* Bottom text */
    g.idle_bottom_text = lv_label_create(g.idle_screen);
    lv_label_set_text(g.idle_bottom_text, "ASR LLM Text Response....");
    lv_obj_set_style_text_color(g.idle_bottom_text, lv_color_hex(0xAAAAAA), 0);
    lv_obj_set_style_text_font(g.idle_bottom_text, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(g.idle_bottom_text, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(g.idle_bottom_text, LV_ALIGN_BOTTOM_MID, 0, -20);
}

static void compass_build(lv_obj_t *parent)
{
    g.compass_container = lv_obj_create(parent);
    lv_obj_remove_style_all(g.compass_container);
    lv_obj_set_size(g.compass_container, CATTLE_SCREEN_WIDTH, CATTLE_SCREEN_HEIGHT);
    lv_obj_clear_flag(g.compass_container, LV_OBJ_FLAG_SCROLLABLE);

    /* Outer dial removed */

    /* Outer cardinal labels removed */

    /* Rotating white circle with N, W, E, S labels - same radius as needle */
    int needle_radius = CATTLE_SCREEN_HEIGHT/2 - 40;  // Same as needle length
    int circle_size = needle_radius * 2;  // Diameter = 2 * radius

    g.compass_labels = lv_obj_create(g.compass_container);
    lv_obj_remove_style_all(g.compass_labels);
    lv_obj_set_size(g.compass_labels, circle_size, circle_size);
    /* Position circle center exactly at CIRCLE_CENTER */
    lv_obj_set_pos(g.compass_labels, CIRCLE_CENTER - needle_radius, CIRCLE_CENTER - needle_radius);
    lv_obj_set_style_radius(g.compass_labels, needle_radius, 0);
    lv_obj_set_style_bg_opa(g.compass_labels, LV_OPA_TRANSP, 0);  // No fill color
    lv_obj_set_style_border_width(g.compass_labels, 35, 0);  // 20px white border
    lv_obj_set_style_border_color(g.compass_labels, lv_color_white(), 0);
    lv_obj_clear_flag(g.compass_labels, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(g.compass_labels, LV_OBJ_FLAG_SCROLLABLE);
    /* Set pivot point to compass center - relative to the circle's position */
    lv_obj_set_style_transform_pivot_x(g.compass_labels, needle_radius, 0);  // needle_radius = half of circle width
    lv_obj_set_style_transform_pivot_y(g.compass_labels, needle_radius, 0);  // needle_radius = half of circle height

    /* N, W, E, S labels positioned on the white circle */
    static const char *rotating_dirs[] = {"N","W","E","S"};
    static const int16_t angles[] = {0, 270, 90, 180}; // N, W, E, S in degrees
    int label_radius = needle_radius - 20; // Distance from center, with some margin

    for(int i=0;i<4;i++){
        lv_obj_t *rotating_lbl = lv_label_create(g.compass_labels);
        lv_label_set_text(rotating_lbl, rotating_dirs[i]);
        lv_obj_set_style_text_color(rotating_lbl, lv_color_black(), 0);
        lv_obj_set_style_text_font(rotating_lbl, &lv_font_montserrat_16, 0);

        /* Position labels on the white circle */
        float angle_rad = angles[i] * M_PI / 180.0f;
        int16_t x = (int16_t)(label_radius * sin(angle_rad));
        int16_t y = (int16_t)(-label_radius * cos(angle_rad));
        lv_obj_align(rotating_lbl, LV_ALIGN_CENTER, x, y);

        /* No individual text rotation - all labels face the same direction */
    }

    /* Needle - thin 2px line */
    g.needle = lv_line_create(g.compass_container);
    static lv_point_precise_t pts[2];
    pts[0].x = CIRCLE_CENTER; pts[0].y = CIRCLE_CENTER;
    pts[1].x = CIRCLE_CENTER; pts[1].y = CIRCLE_CENTER - (CATTLE_SCREEN_HEIGHT/2 - 40);
    lv_line_set_points(g.needle, pts, 2);
    lv_obj_set_style_line_width(g.needle, 2, 0);
    lv_obj_set_style_line_color(g.needle, lv_color_hex(0xff5252), 0);

    /* Orange solid triangle at end of needle - 10px away from line end */
    g.needle_circle = lv_obj_create(g.compass_container);
    lv_obj_set_size(g.needle_circle, 16, 16);
    /* Position triangle at the end of needle line */
    lv_obj_set_pos(g.needle_circle, CIRCLE_CENTER - 8, CIRCLE_CENTER - (CATTLE_SCREEN_HEIGHT/2 - 40) - 20);
    lv_obj_set_style_radius(g.needle_circle, 0, 0);  // No radius for triangle
    lv_obj_set_style_bg_opa(g.needle_circle, LV_OPA_TRANSP, 0);  // Transparent background
    lv_obj_set_style_border_width(g.needle_circle, 0, 0);
    lv_obj_set_style_pad_all(g.needle_circle, 0, 0);
    /* Set pivot point to compass center - relative to the triangle's position */
    lv_obj_set_style_transform_pivot_x(g.needle_circle, 8, 0);  // 8 = half of triangle width
    lv_obj_set_style_transform_pivot_y(g.needle_circle, 8, 0);  // 8 = half of triangle height

    /* Create solid triangle shape using a line object with filled area */
    lv_obj_t *triangle = lv_line_create(g.needle_circle);
    static lv_point_precise_t triangle_pts[4];
    triangle_pts[0].x = 8; triangle_pts[0].y = 14;  // Bottom point (upside down)
    triangle_pts[1].x = 2; triangle_pts[1].y = 2;  // Top left
    triangle_pts[2].x = 14; triangle_pts[2].y = 2;  // Top right
    triangle_pts[3].x = 8; triangle_pts[3].y = 14;  // Back to bottom
    lv_line_set_points(triangle, triangle_pts, 4);
    lv_obj_set_style_line_width(triangle, 0, 0);  // No line width for solid fill
    lv_obj_set_style_line_color(triangle, lv_color_hex(0xFF8800), 0);
    lv_obj_set_style_bg_color(triangle, lv_color_hex(0xFF8800), 0);  // Orange fill
    lv_obj_set_style_bg_opa(triangle, LV_OPA_COVER, 0);  // Solid fill

    /* White spot at dead center */
    lv_obj_t *center_dot = lv_obj_create(g.compass_container);
    lv_obj_set_size(center_dot, 4, 4);
    lv_obj_set_pos(center_dot, CIRCLE_CENTER - 2, CIRCLE_CENTER - 2);
    lv_obj_set_style_radius(center_dot, 2, 0);
    lv_obj_set_style_bg_color(center_dot, lv_color_white(), 0);
    lv_obj_set_style_border_width(center_dot, 0, 0);
    lv_obj_set_style_pad_all(center_dot, 0, 0);

    /* Calibration panel - completely removed */
    g.calib_panel = NULL;
}

static void create_tracking_screen(void)
{
    g.tracking_screen = lv_obj_create(g.screen);
    lv_obj_remove_style_all(g.tracking_screen);
    lv_obj_set_size(g.tracking_screen, CATTLE_SCREEN_WIDTH, CATTLE_SCREEN_HEIGHT);
    lv_obj_set_style_bg_opa(g.tracking_screen, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(g.tracking_screen, LV_OBJ_FLAG_SCROLLABLE);

    compass_build(g.tracking_screen);
}

static void create_settings_panel(void)
{
    /* Backdrop */
    g.settings_backdrop = lv_obj_create(g.screen);
    lv_obj_remove_style_all(g.settings_backdrop);
    lv_obj_set_size(g.settings_backdrop, CATTLE_SCREEN_WIDTH, CATTLE_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(g.settings_backdrop, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(g.settings_backdrop, LV_OPA_30, 0);
    lv_obj_add_flag(g.settings_backdrop, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(g.settings_backdrop, on_settings_backdrop_click, LV_EVENT_CLICKED, NULL);
    /* Don't let backdrop capture keyboard events */
    lv_obj_clear_flag(g.settings_backdrop, LV_OBJ_FLAG_CLICKABLE);

    g.settings_panel = lv_obj_create(g.screen);
    lv_obj_set_size(g.settings_panel, CATTLE_SCREEN_WIDTH, SETTINGS_PANEL_HEIGHT);
    lv_obj_align(g.settings_panel, LV_ALIGN_TOP_MID, 0, -SETTINGS_PANEL_HEIGHT);
    lv_obj_set_style_radius(g.settings_panel, 0, 0);
    lv_obj_set_style_bg_color(g.settings_panel, lv_color_hex(0x182028), 0);
    lv_obj_set_style_bg_opa(g.settings_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g.settings_panel, 0, 0);
    lv_obj_set_style_pad_all(g.settings_panel, 0, 0);
    /* Don't let settings panel capture keyboard events */
    lv_obj_clear_flag(g.settings_panel, LV_OBJ_FLAG_CLICKABLE);

    /* Title */
    lv_obj_t *title = lv_label_create(g.settings_panel);
    lv_label_set_text(title, "Settings");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    /* GPS Status - positioned for larger panel */
    lv_obj_t *gps_label = lv_label_create(g.settings_panel);
    char buf[64];
    snprintf(buf, sizeof(buf), "GPS Status: %d satellites", g.gps_sat_count);
    lv_label_set_text(gps_label, buf);
    lv_obj_set_style_text_color(gps_label, lv_color_hex(0x00AA00), 0);
    lv_obj_set_style_text_align(gps_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(gps_label, LV_ALIGN_CENTER, 0, -80);

    /* Time/Date - positioned for larger panel */
    lv_obj_t *time_label = lv_label_create(g.settings_panel);
    lv_label_set_text(time_label, "Time: 14:30:25\nDate: 2024-01-15");
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(time_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, -40);

    /* Volume slider - positioned for larger panel */
    lv_obj_t *vol_label = lv_label_create(g.settings_panel);
    lv_label_set_text(vol_label, "Volume:");
    lv_obj_set_style_text_color(vol_label, lv_color_white(), 0);
    lv_obj_align(vol_label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *slider = lv_slider_create(g.settings_panel);
    lv_obj_set_width(slider, 250);
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, 30);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x00AA00), LV_PART_INDICATOR);

    /* Draggable tab at bottom */
    g.settings_drag_tab = lv_obj_create(g.settings_panel);
    lv_obj_set_size(g.settings_drag_tab, 60, 8);
    lv_obj_align(g.settings_drag_tab, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(g.settings_drag_tab, lv_color_hex(0x666666), 0);
    lv_obj_set_style_radius(g.settings_drag_tab, 4, 0);
    lv_obj_set_style_border_width(g.settings_drag_tab, 0, 0);
    lv_obj_add_event_cb(g.settings_drag_tab, on_settings_drag, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(g.settings_drag_tab, on_settings_drag, LV_EVENT_RELEASED, NULL);

    /* Close instruction */
    lv_obj_t *close_hint = lv_label_create(g.settings_panel);
    lv_label_set_text(close_hint, "Press 'S' to close");
    lv_obj_set_style_text_color(close_hint, lv_color_hex(0xAAAAAA), 0);
    lv_obj_align(close_hint, LV_ALIGN_BOTTOM_MID, 0, -30);
}

static void create_sos_screen(void)
{
    g.sos_screen = lv_obj_create(g.screen);
    lv_obj_remove_style_all(g.sos_screen);
    lv_obj_set_size(g.sos_screen, CATTLE_SCREEN_WIDTH, CATTLE_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(g.sos_screen, lv_color_hex(0x2a1a1a), 0);
    lv_obj_set_style_bg_opa(g.sos_screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(g.sos_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g.sos_screen, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *title = lv_label_create(g.sos_screen);
    lv_label_set_text(title, "SOS Active\n\nPress 'X' to cancel");
    lv_obj_set_style_text_color(title, lv_color_hex(0xffeaea), 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(title);

    /* Hold progress ring (shown during long press on main/root) */
    g.sos_hold_ring = lv_arc_create(g.screen);
    lv_obj_set_size(g.sos_hold_ring, CATTLE_SCREEN_WIDTH - 90, CATTLE_SCREEN_HEIGHT - 90);
    lv_obj_center(g.sos_hold_ring);
    lv_arc_set_rotation(g.sos_hold_ring, 270);
    lv_arc_set_bg_angles(g.sos_hold_ring, 0, 360);
    lv_arc_set_value(g.sos_hold_ring, 0);
    lv_obj_set_style_arc_color(g.sos_hold_ring, lv_color_hex(0x333a), LV_PART_MAIN);
    lv_obj_set_style_arc_width(g.sos_hold_ring, 10, LV_PART_MAIN);
    lv_obj_set_style_arc_color(g.sos_hold_ring, lv_color_hex(0xff5555), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(g.sos_hold_ring, 12, LV_PART_INDICATOR);
    lv_obj_add_flag(g.sos_hold_ring, LV_OBJ_FLAG_HIDDEN);
}

static void show_idle(void)
{
    lv_obj_add_flag(g.tracking_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g.sos_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g.settings_panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g.settings_backdrop, LV_OBJ_FLAG_HIDDEN);

    /* Clear hidden flag and position for animation */
    lv_obj_clear_flag(g.idle_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_x(g.idle_screen, -CATTLE_SCREEN_WIDTH);  /* Start from left side */

    /* Animate idle screen sliding in from left with settings-style animation */
    lv_anim_t a; lv_anim_init(&a);
    lv_anim_set_var(&a, g.idle_screen);
    lv_anim_set_values(&a, -CATTLE_SCREEN_WIDTH, 0);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_time(&a, 400);  /* Match settings panel timing */
    lv_anim_set_early_apply(&a, true);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);  /* Same easing as settings */
    lv_anim_start(&a);
}

static void show_tracking(void)
{
    lv_obj_add_flag(g.idle_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g.sos_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g.settings_panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g.settings_backdrop, LV_OBJ_FLAG_HIDDEN);

    /* Clear hidden flag and position for animation */
    lv_obj_clear_flag(g.tracking_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_x(g.tracking_screen, CATTLE_SCREEN_WIDTH);  /* Start from right side */

    /* Animate tracking screen sliding in from right with settings-style animation */
    lv_anim_t a; lv_anim_init(&a);
    lv_anim_set_var(&a, g.tracking_screen);
    lv_anim_set_values(&a, CATTLE_SCREEN_WIDTH, 0);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_time(&a, 400);  /* Match settings panel timing */
    lv_anim_set_early_apply(&a, true);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);  /* Same easing as settings */
    lv_anim_start(&a);
}

static void show_sos_alert(void)
{
    g.sos_active = true;
    lv_obj_add_flag(g.sos_hold_ring, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g.idle_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g.tracking_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(g.sos_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g.settings_panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g.settings_backdrop, LV_OBJ_FLAG_HIDDEN);
}

static void hide_sos_alert(void)
{
    g.sos_active = false;
    lv_obj_add_flag(g.sos_screen, LV_OBJ_FLAG_HIDDEN);
}

static void slide_settings(bool open)
{
    if(open){
        lv_obj_clear_flag(g.settings_backdrop, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g.settings_panel, LV_OBJ_FLAG_HIDDEN);

        /* Enhanced opening animation with easing */
        lv_anim_t a; lv_anim_init(&a);
        lv_anim_set_var(&a, g.settings_panel);
        lv_anim_set_values(&a, -SETTINGS_PANEL_HEIGHT, 0);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_time(&a, 400);  /* Slightly longer for smoother animation */
        lv_anim_set_early_apply(&a, true);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);  /* Ease out for natural feel */
        lv_anim_start(&a);

        /* Fade in backdrop */
        lv_anim_t b; lv_anim_init(&b);
        lv_anim_set_var(&b, g.settings_backdrop);
        lv_anim_set_values(&b, LV_OPA_TRANSP, LV_OPA_30);
        lv_anim_set_exec_cb(&b, (lv_anim_exec_xcb_t)lv_obj_set_style_bg_opa);
        lv_anim_set_time(&b, 300);
        lv_anim_set_early_apply(&b, true);
        lv_anim_start(&b);

    } else {
        /* Enhanced closing animation with easing */
        lv_anim_t a; lv_anim_init(&a);
        lv_anim_set_var(&a, g.settings_panel);
        lv_anim_set_values(&a, 0, -SETTINGS_PANEL_HEIGHT);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_time(&a, 350);  /* Slightly longer for smoother animation */
        lv_anim_set_early_apply(&a, true);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in);  /* Ease in for natural feel */
        lv_anim_set_ready_cb(&a, on_settings_close_anim_ready);
        lv_anim_start(&a);

        /* Fade out backdrop */
        lv_anim_t b; lv_anim_init(&b);
        lv_anim_set_var(&b, g.settings_backdrop);
        lv_anim_set_values(&b, LV_OPA_30, LV_OPA_TRANSP);
        lv_anim_set_exec_cb(&b, (lv_anim_exec_xcb_t)lv_obj_set_style_bg_opa);
        lv_anim_set_time(&b, 300);
        lv_anim_set_early_apply(&b, true);
        lv_anim_start(&b);
    }
}

static void on_settings_drag(lv_event_t *e)
{
    lv_indev_t *indev = lv_indev_get_act();
    lv_point_t point;
    lv_indev_get_point(indev, &point);

    /* Get current panel position */
    lv_coord_t current_y = lv_obj_get_y(g.settings_panel);

    /* Calculate drag distance from initial position */
    static lv_coord_t start_y = 0;
    static bool drag_started = false;

    if(lv_event_get_code(e) == LV_EVENT_PRESSING) {
        if(!drag_started) {
            start_y = current_y;
            drag_started = true;
        }

        /* Calculate new position based on drag */
        lv_coord_t drag_delta = point.y - start_y;
        lv_coord_t new_y = start_y + drag_delta;

        /* Limit movement to reasonable bounds */
        if(new_y < -SETTINGS_PANEL_HEIGHT) new_y = -SETTINGS_PANEL_HEIGHT;
        if(new_y > 0) new_y = 0;

        /* Update panel position in real-time */
        lv_obj_set_y(g.settings_panel, new_y);

        /* If dragged down significantly, close the panel */
        if(new_y > SETTINGS_PANEL_HEIGHT / 3) {
            printf("Drag down - closing settings\n");
            slide_settings(false);
            drag_started = false;
        }
    } else if(lv_event_get_code(e) == LV_EVENT_RELEASED) {
        if(drag_started) {
            /* Snap back to original position if not closed */
            if(current_y > -SETTINGS_PANEL_HEIGHT / 3) {
                printf("Drag released - snapping back\n");
                lv_obj_set_y(g.settings_panel, 0);
            }
            drag_started = false;
        }
    }
}

static void on_gesture(lv_event_t *e)
{
    lv_indev_t *indev = lv_indev_get_act();
    lv_indev_type_t indev_type = lv_indev_get_type(indev);

    if(indev_type == LV_INDEV_TYPE_POINTER) {
        lv_dir_t dir = lv_indev_get_gesture_dir(indev);

        switch(dir) {
            case LV_DIR_TOP:  // Swipe up - close settings
                if(!lv_obj_has_flag(g.settings_panel, LV_OBJ_FLAG_HIDDEN)) {
                    printf("Swipe up - closing settings with animation\n");
                    slide_settings(false);
                }
                break;
            case LV_DIR_BOTTOM:  // Swipe down - open settings
                if(lv_obj_has_flag(g.settings_panel, LV_OBJ_FLAG_HIDDEN)) {
                    printf("Swipe down - opening settings\n");
                    slide_settings(true);
                }
                break;
            case LV_DIR_LEFT:  // Swipe left - tracking screen
                printf("Swipe left - showing tracking\n");
                show_tracking();
                break;
            case LV_DIR_RIGHT:  // Swipe right - idle screen
                printf("Swipe right - showing idle\n");
                show_idle();
                break;
            default:
                break;
        }
    }
}

static void on_settings_backdrop_click(lv_event_t *e)
{
    (void)e;
    slide_settings(false);
}

static void on_keyboard(lv_event_t *e)
{
    uint32_t key = lv_event_get_key(e);
    printf("Key pressed: %d (char: %c)\n", key, (char)key);

    switch(key) {
        case 'i': case 'I':  // Idle screen
            printf("Key I - showing idle\n");
            show_idle();
            break;
        case 't': case 'T':  // Tracking screen
            printf("Key T - showing tracking\n");
            show_tracking();
            break;
        case 's': case 'S':  // Settings
            printf("Key S - toggling settings\n");
            if(lv_obj_has_flag(g.settings_panel, LV_OBJ_FLAG_HIDDEN)) {
                slide_settings(true);
            } else {
                slide_settings(false);
            }
            break;
        case 'x': case 'X':  // Cancel SOS
            printf("Key X - canceling SOS\n");
            if(g.sos_active) {
                hide_sos_alert();
            }
            break;
        case ' ':  // Space for SOS
            printf("Space - triggering SOS\n");
            if(!g.sos_active) {
                show_sos_alert();
            }
            break;
        default:
            printf("Unhandled key: %d\n", key);
            break;
    }
}

static void on_pressed(lv_event_t *e)
{
    (void)e;
    if(g.sos_active) return;
    g.sos_pressed_start_ms = lv_tick_get();
    lv_arc_set_value(g.sos_hold_ring, 0);
    lv_obj_clear_flag(g.sos_hold_ring, LV_OBJ_FLAG_HIDDEN);
}

static void on_pressing(lv_event_t *e)
{
    (void)e;
    if(g.sos_active) return;
    if(g.sos_pressed_start_ms == 0) return;
    uint32_t elapsed = lv_tick_elaps(g.sos_pressed_start_ms);
    int32_t pct = (int32_t)((elapsed * 100) / SOS_HOLD_MS);
    if(pct > 100) pct = 100;
    lv_arc_set_value(g.sos_hold_ring, pct);
    if(elapsed >= SOS_HOLD_MS){
        g.sos_pressed_start_ms = 0;
        show_sos_alert();
    }
}

static void on_released(lv_event_t *e)
{
    (void)e;
    if(!g.sos_active){
        lv_obj_add_flag(g.sos_hold_ring, LV_OBJ_FLAG_HIDDEN);
    }
    g.sos_pressed_start_ms = 0;
}

static void on_sos_cancel(lv_event_t *e)
{
    (void)e;
    hide_sos_alert();
}

/* Calibration function removed - no longer needed */

static void compass_update(float yaw_deg)
{
    /* Rotate needle */
    lv_obj_set_style_transform_angle(g.needle, (int16_t)(yaw_deg * 10), 0);
    lv_obj_set_style_transform_pivot_x(g.needle, CIRCLE_CENTER, 0);
    lv_obj_set_style_transform_pivot_y(g.needle, CIRCLE_CENTER, 0);

}

static void on_settings_close_anim_ready(lv_anim_t * anim)
{
    (void)anim;
    lv_obj_add_flag(g.settings_panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g.settings_backdrop, LV_OBJ_FLAG_HIDDEN);

    /* Ensure focus returns to keyboard handler */
    if(g.focus_obj) {
        lv_group_focus_obj(g.focus_obj);
    }
}

static float wrap_deg(float d){
    while(d < 0) d += 360.f; while(d >= 360.f) d -= 360.f; return d;
}

static void on_tick(lv_timer_t *t)
{
    (void)t;
    /* Dummy motion: yaw slowly changes */
    g.yaw_deg = wrap_deg(g.yaw_deg + 1.2f);
    /* Always update compass since we start calibrated */
    compass_update(g.yaw_deg);
}

/* Function to update idle screen bottom text */
void update_idle_bottom_text(const char* text)
{
    if(g.idle_bottom_text) {
        lv_label_set_text(g.idle_bottom_text, text);
    }
}
