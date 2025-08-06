/**
 * @file lv_demo_ai_pocket_pet.c
 * AI Pocket Pet Demo for LVGL
 *
 * This module implements a Tamagotchi-style virtual pet interface using LVGL.
 * Features include pet care, statistics tracking, and menu navigation.
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_demo_ai_pocket_pet.h"
#include "lv_keyboard_widget.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

// Custom menu icons
#include "data/icons/menu_info_icon.c"
#include "data/icons/menu_eat_icon.c"
#include "data/icons/menu_sleep_icon.c"
#include "data/icons/menu_sick_icon.c"
#include "data/icons/menu_toilet_icon.c"
#include "data/icons/menu_camera_icon.c"

// Network status icons
#include "data/icons/wifi_1_bar_icon.c"
#include "data/icons/wifi_2_bar_icon.c"
#include "data/icons/wifi_3_bar_icon.c"
#include "data/icons/wifi_off_icon.c"
#include "data/icons/wifi_find_icon.c"
#include "data/icons/wifi_add_icon.c"
#include "data/icons/4g_logo_icon.c"
#include "data/icons/cellular_1_bar_icon.c"
#include "data/icons/cellular_2_bar_icon.c"
#include "data/icons/cellular_3_bar_icon.c"
#include "data/icons/cellular_off_icon.c"
#include "data/icons/cellular_connected_no_internet_icon.c"

// Battery icons
#include "data/icons/battery_0_icon.c"
#include "data/icons/battery_1_icon.c"
#include "data/icons/battery_2_icon.c"
#include "data/icons/battery_3_icon.c"
#include "data/icons/battery_4_icon.c"
#include "data/icons/battery_5_icon.c"
#include "data/icons/battery_full_icon.c"
#include "data/icons/battery_charging_icon.c"

// Pet animation
#include "data/ducky/ducky_walk.c"
#include "data/ducky/ducky_walk_to_left.c"
#include "data/ducky/ducky_blink.c"
LV_IMG_DECLARE(ducky_walk);
LV_IMG_DECLARE(ducky_walk_to_left);
LV_IMG_DECLARE(ducky_blink);

// Network status icon declarations
LV_IMG_DECLARE(wifi_1_bar_icon);
LV_IMG_DECLARE(wifi_2_bar_icon);
LV_IMG_DECLARE(wifi_3_bar_icon);
LV_IMG_DECLARE(wifi_off_icon);
LV_IMG_DECLARE(wifi_find_icon);
LV_IMG_DECLARE(wifi_add_icon);
LV_IMG_DECLARE(four_g_logo_icon);
LV_IMG_DECLARE(cellular_1_bar_icon);
LV_IMG_DECLARE(cellular_2_bar_icon);
LV_IMG_DECLARE(cellular_3_bar_icon);
LV_IMG_DECLARE(cellular_off_icon);
LV_IMG_DECLARE(cellular_connected_no_internet_icon);

// Battery icon declarations
LV_IMG_DECLARE(battery_0_icon);
LV_IMG_DECLARE(battery_1_icon);
LV_IMG_DECLARE(battery_2_icon);
LV_IMG_DECLARE(battery_3_icon);
LV_IMG_DECLARE(battery_4_icon);
LV_IMG_DECLARE(battery_5_icon);
LV_IMG_DECLARE(battery_full_icon);
LV_IMG_DECLARE(battery_charging_icon);

/*********************
 *      DEFINES
 *********************/
#define STATUS_BAR_HEIGHT 24
#define BOTTOM_MENU_HEIGHT 26
#define PET_AREA_HEIGHT (AI_PET_SCREEN_HEIGHT - STATUS_BAR_HEIGHT - BOTTOM_MENU_HEIGHT)

// Toast message constants
#define TOAST_PADDING 20
#define TOAST_MAX_WIDTH (AI_PET_SCREEN_WIDTH - 40)
#define TOAST_MIN_HEIGHT 60
#define TOAST_ANIMATION_DURATION 300
#define TOAST_DEFAULT_DELAY 3000

// UI Constants
#define MENU_BUTTON_COUNT 6
#define MENU_BUTTON_SIZE 24
#define MENU_BUTTON_SPACING 30
#define MENU_BUTTON_START_X (AI_PET_SCREEN_WIDTH - 225)
#define SUB_MENU_PADDING 10
#define SUB_MENU_TITLE_OFFSET 10
#define SUB_MENU_LIST_OFFSET 40
#define STAT_CONTAINER_HEIGHT 30
#define STAT_CONTAINER_WIDTH (AI_PET_SCREEN_WIDTH - 40)
#define SEPARATOR_HEIGHT 2

// LVGL key codes
#define KEY_UP    17  // LV_KEY_UP
#define KEY_LEFT  20  // LV_KEY_LEFT
#define KEY_DOWN  18  // LV_KEY_DOWN
#define KEY_RIGHT 19  // LV_KEY_RIGHT
#define KEY_ENTER 10  // LV_KEY_ENTER
#define KEY_ESC   27  // LV_KEY_ESC
#define KEY_I     105 // 'i' key

#define LV_IMG_DECLARE(var_name) extern const lv_image_dsc_t var_name;
// Pet animation constants
#define PET_ANIMATION_INTERVAL 20
#define PET_MOVEMENT_INTERVAL 50   // Natural movement timing
#define PET_MOVEMENT_STEP 2        // Smooth movement step
#define PET_MOVEMENT_LIMIT 80      // Movement boundaries
#define PET_BLINK_INTERVAL 3000    // Blink every 3 seconds
#define PET_WALK_DURATION_MIN 2000 // Minimum walk duration (ms)
#define PET_WALK_DURATION_MAX 8000 // Maximum walk duration (ms)
#define PET_IDLE_DURATION_MIN 3000 // Minimum idle duration (ms)
#define PET_IDLE_DURATION_MAX 10000 // Maximum idle duration (ms)

// Pet stats constants
#define MAX_STAT_VALUE 100
#define MIN_WEIGHT_KG 1.0f
#define MAX_WEIGHT_KG 5.0f
#define WEIGHT_INCREMENT 0.1f

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_obj_t *screen;
    lv_obj_t *status_bar;
    lv_obj_t *wifi_icon;
    lv_obj_t *four_g_logo_icon;
    lv_obj_t *network_icon;
    lv_obj_t *battery_icon;
    lv_obj_t *pet_area;
    lv_obj_t *pet_image_walk;
    lv_obj_t *pet_image_walk_left;
    lv_obj_t *pet_image_blink;
    lv_obj_t *current_pet_image; // Points to the currently active image
    lv_obj_t *bottom_menu;
    lv_obj_t *menu_buttons[MENU_BUTTON_COUNT];
    lv_obj_t *sub_menu;
    lv_obj_t *sub_menu_list;

    // Toast message components
    lv_obj_t *toast_container;
    lv_obj_t *toast_label;
    lv_timer_t *toast_timer;

    ai_pet_state_t pet_state;
    ai_pet_menu_t current_menu;
    uint8_t selected_button;
    uint8_t sub_menu_selection;

    ai_pet_stats_t pet_stats;

    lv_timer_t *pet_animation_timer;
    lv_timer_t *pet_movement_timer;

    // Pet movement state
    int16_t pet_x_pos;
    int8_t pet_direction;  // 1 = right, -1 = left
    uint32_t pet_state_timer;
    uint32_t pet_state_duration;
    bool pet_is_walking;

    // Network status tracking
    uint8_t wifi_signal_strength;  // 0 = off, 1-3 = bars, 4 = find, 5 = add
    uint8_t cellular_signal_strength;  // 0 = off, 1-3 = bars, 4 = no internet
    bool cellular_connected;

    // Battery status tracking
    uint8_t battery_level;  // 0-6 (0 = empty, 5 = 5 bars, 6 = full)
    bool battery_charging;
} ai_pet_demo_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

// UI Creation Functions
static void create_status_bar(ai_pet_demo_t *demo);
static void create_pet_area(ai_pet_demo_t *demo);
static void create_bottom_menu(ai_pet_demo_t *demo);
static void create_sub_menu(ai_pet_demo_t *demo);

// Animation Functions
static void pet_animation_cb(lv_timer_t *timer);
static void pet_movement_cb(lv_timer_t *timer);

static void switch_pet_animation(lv_obj_t *new_animation);

// Event Handler Functions
static void menu_button_event_cb(lv_event_t *e);
static void sub_menu_event_cb(lv_event_t *e);
static void keyboard_event_cb(lv_event_t *e);
static void keyboard_callback(keyboard_result_t result, const char *text, void *user_data);

// Menu Management Functions
static void show_info_menu(ai_pet_demo_t *demo);
static void show_food_menu(ai_pet_demo_t *demo);
static void show_bath_menu(ai_pet_demo_t *demo);
static void show_health_menu(ai_pet_demo_t *demo);
static void show_sleep_menu(ai_pet_demo_t *demo);
static void show_video_menu(ai_pet_demo_t *demo);
static void hide_sub_menu(ai_pet_demo_t *demo);
static void show_keyboard_for_pet_name(ai_pet_demo_t *demo);

// Info Menu Helper Functions
static void create_pet_name_display(ai_pet_demo_t *demo);
static void create_pet_stats_displays(ai_pet_demo_t *demo);
static void create_separator(void);
static void create_actions_section(void);

// Initialization Functions
static void init_demo_data(void);
static void create_main_screen(void);
static void start_animation_timers(void);

// Input Handling Functions
static void handle_main_menu_navigation(uint32_t key);
static void handle_sub_menu_navigation(uint32_t key);
static void handle_menu_selection(void);
static void handle_sub_menu_selection(void);
static void handle_ai_function(void);

// Utility Functions
static void update_button_selection(uint8_t old_selection, uint8_t new_selection);
static void init_pet_stats(ai_pet_stats_t *stats);
static void update_pet_stats_display(ai_pet_demo_t *demo);
static void update_sub_menu_selection(uint8_t old_selection, uint8_t new_selection);
static uint32_t find_action_items_start(void);
static void create_stat_display_item(lv_obj_t *parent, const char *label, const char *value);
static void highlight_first_sub_menu_item(ai_pet_demo_t *demo);
static void create_sub_menu_with_items(ai_pet_demo_t *demo, const char *title, const char *symbols[], const char *items[], uint8_t item_count);

// Toast Message Functions
static void create_toast_message(ai_pet_demo_t *demo);
static void show_toast_message(const char *message, uint32_t delay_ms);
static void hide_toast_message(void);
static void toast_timer_cb(lv_timer_t *timer);
static void toast_anim_ready_cb(lv_anim_t *a);

// Network status functions
static void update_wifi_icon(ai_pet_demo_t *demo, uint8_t signal_strength);
static void update_cellular_icon(ai_pet_demo_t *demo, uint8_t signal_strength, bool connected);
static void update_network_status_bar(ai_pet_demo_t *demo);
static const lv_img_dsc_t* get_wifi_icon_by_strength(uint8_t strength);
static const lv_img_dsc_t* get_cellular_icon_by_strength(uint8_t strength, bool connected);

// Battery status functions
static void update_battery_icon(ai_pet_demo_t *demo, uint8_t level, bool charging);
static const lv_img_dsc_t* get_battery_icon_by_level(uint8_t level, bool charging);

/**********************
 *  STATIC VARIABLES
 **********************/
static ai_pet_demo_t demo_data;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initializes the demo data structure with default values
 */
static void init_demo_data(void)
{
    memset(&demo_data, 0, sizeof(ai_pet_demo_t));
    demo_data.pet_state = AI_PET_STATE_IDLE;
    demo_data.current_menu = AI_PET_MENU_MAIN;
    demo_data.selected_button = 0;
    demo_data.sub_menu_selection = 0;
    demo_data.pet_x_pos = 0;
    demo_data.pet_direction = 1;  // Start facing right
    demo_data.pet_is_walking = false;
    demo_data.pet_state_timer = 0;
    demo_data.pet_state_duration = PET_IDLE_DURATION_MIN + (rand() % (PET_IDLE_DURATION_MAX - PET_IDLE_DURATION_MIN));
    init_pet_stats(&demo_data.pet_stats);
}

/**
 * Creates and configures the main screen
 */
static void create_main_screen(void)
{
    demo_data.screen = lv_obj_create(NULL);
    lv_obj_set_size(demo_data.screen, AI_PET_SCREEN_WIDTH, AI_PET_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(demo_data.screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(demo_data.screen, LV_OPA_COVER, 0);
    lv_screen_load(demo_data.screen);

    // Add keyboard event handler to the screen
    lv_obj_add_event_cb(demo_data.screen, keyboard_event_cb, LV_EVENT_KEY, NULL);

    // Make sure the screen can receive keyboard focus
    lv_group_add_obj(lv_group_get_default(), demo_data.screen);

    // Add horizontal line across the screen, 3px thick, positioned 1/3 from bottom
    lv_obj_t *horizontal_line = lv_obj_create(demo_data.screen);
    lv_obj_set_size(horizontal_line, AI_PET_SCREEN_WIDTH, 2);
    lv_obj_align(horizontal_line, LV_ALIGN_TOP_LEFT, 0, 112); // 168 * (2/3) = 112 pixels from top
    lv_obj_set_style_bg_color(horizontal_line, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(horizontal_line, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(horizontal_line, 0, 0);
    lv_obj_set_style_pad_all(horizontal_line, 0, 0);
}

/**
 * Starts the pet animation timers
 */
static void start_animation_timers(void)
{
    demo_data.pet_animation_timer = lv_timer_create(pet_animation_cb, PET_ANIMATION_INTERVAL, &demo_data);
    demo_data.pet_movement_timer = lv_timer_create(pet_movement_cb, PET_MOVEMENT_INTERVAL, &demo_data);
}

/**
 * Main demo initialization function
 */
void lv_demo_ai_pocket_pet(void)
{
    // Initialize random seed for natural movement
    srand(time(NULL));

    // Initialize demo data
    init_demo_data();

    // Initialize keyboard widget
    lv_keyboard_widget_init();

    // Create main screen
    create_main_screen();

    // Create UI components
    create_status_bar(&demo_data);
    create_pet_area(&demo_data);
    create_bottom_menu(&demo_data);
    create_sub_menu(&demo_data);
    create_toast_message(&demo_data);

    // Start pet animation timers
    start_animation_timers();

    // Test network status icons - demonstrate different states
    printf("Initializing network status icons...\n");

    // Set initial WiFi to 3 bars and cellular to 2 bars with connection
    lv_demo_ai_pocket_pet_set_wifi_strength(3);
    lv_demo_ai_pocket_pet_set_cellular_status(2, true);

    // Show a toast message to indicate the demo is ready
    lv_demo_ai_pocket_pet_show_toast("Network icons initialized!", 2000);
}

void lv_demo_ai_pocket_pet_handle_input(uint32_t key)
{
    // Check if keyboard widget is active - if so, route input to keyboard
    if (lv_keyboard_widget_is_active()) {
        lv_keyboard_widget_handle_input(key);
        return;
    }

    printf("Key pressed: %d (UP:%d LEFT:%d DOWN:%d RIGHT:%d ENTER:%d ESC:%d I:%d)\n",
           key, KEY_UP, KEY_LEFT, KEY_DOWN, KEY_RIGHT, KEY_ENTER, KEY_ESC, KEY_I);

    switch(key) {
        case KEY_UP:
            printf("UP key pressed - navigating up\n");
            if(demo_data.current_menu == AI_PET_MENU_MAIN) {
                handle_main_menu_navigation(key);
            } else {
                handle_sub_menu_navigation(key);
            }
            break;

        case KEY_DOWN:
            printf("DOWN key pressed - navigating down\n");
            if(demo_data.current_menu == AI_PET_MENU_MAIN) {
                handle_main_menu_navigation(key);
            } else {
                handle_sub_menu_navigation(key);
            }
            break;

        case KEY_LEFT:
            printf("LEFT/A key pressed - navigating left\n");
            handle_main_menu_navigation(key);
            break;

        case KEY_RIGHT:
            printf("RIGHT/D key pressed - navigating right\n");
            handle_main_menu_navigation(key);
            break;

        case KEY_ENTER:
            if(demo_data.current_menu == AI_PET_MENU_MAIN) {
                handle_menu_selection();
            } else {
                handle_sub_menu_selection();
            }
            break;

        case KEY_ESC:
            if(demo_data.current_menu != AI_PET_MENU_MAIN) {
                hide_sub_menu(&demo_data);
            }
            break;

        case KEY_I:
            printf("I key pressed - AI function invoked\n");
            handle_ai_function();
            break;

        // Battery icon testing keys only
        case 97: // 'a' key - Battery 0 (empty)
            printf("A key pressed - Setting battery to empty\n");
            lv_demo_ai_pocket_pet_set_battery_status(0, false);
            lv_demo_ai_pocket_pet_show_toast("Battery: Empty", 1000);
            break;

        case 115: // 's' key - Battery 1
            printf("S key pressed - Setting battery to 1 bar\n");
            lv_demo_ai_pocket_pet_set_battery_status(1, false);
            lv_demo_ai_pocket_pet_show_toast("Battery: 1 bar", 1000);
            break;

        case 100: // 'd' key - Battery 2
            printf("D key pressed - Setting battery to 2 bars\n");
            lv_demo_ai_pocket_pet_set_battery_status(2, false);
            lv_demo_ai_pocket_pet_show_toast("Battery: 2 bars", 1000);
            break;

        case 102: // 'f' key - Battery 3
            printf("F key pressed - Setting battery to 3 bars\n");
            lv_demo_ai_pocket_pet_set_battery_status(3, false);
            lv_demo_ai_pocket_pet_show_toast("Battery: 3 bars", 1000);
            break;

        case 103: // 'g' key - Battery 4
            printf("G key pressed - Setting battery to 4 bars\n");
            lv_demo_ai_pocket_pet_set_battery_status(4, false);
            lv_demo_ai_pocket_pet_show_toast("Battery: 4 bars", 1000);
            break;

        case 104: // 'h' key - Battery 5 (5 bars)
            printf("H key pressed - Setting battery to 5 bars\n");
            lv_demo_ai_pocket_pet_set_battery_status(5, false);
            lv_demo_ai_pocket_pet_show_toast("Battery: 5 bars", 1000);
            break;

        case 106: // 'j' key - Battery 6 (full)
            printf("J key pressed - Setting battery to full\n");
            lv_demo_ai_pocket_pet_set_battery_status(6, false);
            lv_demo_ai_pocket_pet_show_toast("Battery: Full", 1000);
            break;

        case 99: // 'c' key - Battery charging
            printf("C key pressed - Setting battery to charging\n");
            lv_demo_ai_pocket_pet_set_battery_status(3, true);
            lv_demo_ai_pocket_pet_show_toast("Battery: Charging", 1000);
            break;

        default:
            printf("Unhandled key: %d\n", key);
            if(key > 0) {
                printf("Key press detected but not handled: %d\n", key);
            }
            break;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Creates the status bar with WiFi, network, and battery icons
 */
static void create_status_bar(ai_pet_demo_t *demo)
{
    demo->status_bar = lv_obj_create(demo->screen);
    lv_obj_set_size(demo->status_bar, AI_PET_SCREEN_WIDTH, STATUS_BAR_HEIGHT);
    lv_obj_align(demo->status_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(demo->status_bar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(demo->status_bar, 0, 0);
    lv_obj_set_style_pad_all(demo->status_bar, 2, 0);

    // Disable scrolling for status bar
    lv_obj_clear_flag(demo->status_bar, LV_OBJ_FLAG_SCROLLABLE);

    // WiFi icon (image widget)
    demo->wifi_icon = lv_img_create(demo->status_bar);
    lv_obj_set_size(demo->wifi_icon, 24, 24);
    lv_obj_align(demo->wifi_icon, LV_ALIGN_LEFT_MID, 5, 0);

    // 4G logo icon (static, 24px) -- moved before cellular icon
    demo->four_g_logo_icon = lv_img_create(demo->status_bar);
    lv_obj_set_size(demo->four_g_logo_icon, 24, 24);
    lv_obj_align(demo->four_g_logo_icon, LV_ALIGN_LEFT_MID, 35, 0);
    lv_img_set_src(demo->four_g_logo_icon, &four_g_logo_icon);

    // Network icon (cellular signal) -- moved after 4G icon
    demo->network_icon = lv_img_create(demo->status_bar);
    lv_obj_set_size(demo->network_icon, 24, 24);
    lv_obj_align(demo->network_icon, LV_ALIGN_LEFT_MID, 55, 0);

    // Battery icon (image widget)
    demo->battery_icon = lv_img_create(demo->status_bar);
    lv_obj_set_size(demo->battery_icon, 24, 24);
    lv_obj_align(demo->battery_icon, LV_ALIGN_RIGHT_MID, -5, 0);

    // Initialize network status
    demo->wifi_signal_strength = 4;  // Default to not connected
    demo->cellular_signal_strength = 4;  // Default to not connected
    demo->cellular_connected = false;    // Default to not connected

    // Initialize battery status
    demo->battery_level = 5;  // Default to full battery
    demo->battery_charging = false;  // Default to not charging

    // Set initial icons
    update_network_status_bar(demo);
    update_battery_icon(demo, demo->battery_level, demo->battery_charging);
}

/**
 * Creates the pet display area with the pet sprite
 */
static void create_pet_area(ai_pet_demo_t *demo)
{
    demo->pet_area = lv_obj_create(demo->screen);
    // Use full screen height minus status bar and bottom menu
    lv_obj_set_size(demo->pet_area, AI_PET_SCREEN_WIDTH, AI_PET_SCREEN_HEIGHT - STATUS_BAR_HEIGHT - BOTTOM_MENU_HEIGHT);
    lv_obj_align(demo->pet_area, LV_ALIGN_TOP_MID, 0, STATUS_BAR_HEIGHT);
    lv_obj_set_style_bg_opa(demo->pet_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(demo->pet_area, 0, 0);
    lv_obj_set_style_pad_all(demo->pet_area, 0, 0); // Remove padding to maximize space

    // Disable scrolling for pet area
    lv_obj_clear_flag(demo->pet_area, LV_OBJ_FLAG_SCROLLABLE);

    // Create a container for the GIF widgets to constrain rendering area
    lv_obj_t *gif_container = lv_obj_create(demo->pet_area);
    lv_obj_set_size(gif_container, 159, 164); // Exact GIF dimensions
    lv_obj_align(gif_container, LV_ALIGN_CENTER, 0, -5); // Center with slight upward offset
    lv_obj_set_style_bg_opa(gif_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(gif_container, 0, 0);
    lv_obj_set_style_pad_all(gif_container, 0, 0);
    lv_obj_clear_flag(gif_container, LV_OBJ_FLAG_SCROLLABLE);

        // Create three separate GIF widgets for smooth animation transitions
    // This prevents black flashing by avoiding source switching

    // Walk right animation
    demo->pet_image_walk = lv_gif_create(gif_container);
    lv_gif_set_src(demo->pet_image_walk, &ducky_walk);
    lv_obj_align(demo->pet_image_walk, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(demo->pet_image_walk, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(demo->pet_image_walk, 159, 164);
    lv_obj_set_style_bg_opa(demo->pet_image_walk, LV_OPA_TRANSP, 0);

    // Walk left animation
    demo->pet_image_walk_left = lv_gif_create(gif_container);
    lv_gif_set_src(demo->pet_image_walk_left, &ducky_walk_to_left);
    lv_obj_align(demo->pet_image_walk_left, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(demo->pet_image_walk_left, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(demo->pet_image_walk_left, 159, 164);
    lv_obj_set_style_bg_opa(demo->pet_image_walk_left, LV_OPA_TRANSP, 0);

    // Blink animation
    demo->pet_image_blink = lv_gif_create(gif_container);
    lv_gif_set_src(demo->pet_image_blink, &ducky_blink);
    lv_obj_align(demo->pet_image_blink, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(demo->pet_image_blink, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(demo->pet_image_blink, 159, 164);
    lv_obj_set_style_bg_opa(demo->pet_image_blink, LV_OPA_TRANSP, 0);

    // Set initial active image and hide others
    demo->current_pet_image = demo->pet_image_blink;
    lv_obj_add_flag(demo->pet_image_walk, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(demo->pet_image_walk_left, LV_OBJ_FLAG_HIDDEN);

    printf("Ducky GIF animation loaded - full resolution: %dx%d\n", ducky_walk.header.w, ducky_walk.header.h);
}

/**
 * Creates the bottom menu with navigation buttons
 */
static void create_bottom_menu(ai_pet_demo_t *demo)
{
    demo->bottom_menu = lv_obj_create(demo->screen);
    lv_obj_set_size(demo->bottom_menu, AI_PET_SCREEN_WIDTH, BOTTOM_MENU_HEIGHT);
    lv_obj_align(demo->bottom_menu, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(demo->bottom_menu, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(demo->bottom_menu, 0, 0);
    lv_obj_set_style_pad_all(demo->bottom_menu, 2, 0);

    // Custom menu icons - using image objects instead of symbols
    const lv_img_dsc_t *menu_icons[] = {
        &info_icon,
        &eat_icon,
        &toilet_icon,
        &sick_icon,
        &sleep_icon,
        &camera_icon
    };

    for(int i = 0; i < MENU_BUTTON_COUNT; i++) {
        demo->menu_buttons[i] = lv_btn_create(demo->bottom_menu);
        lv_obj_set_size(demo->menu_buttons[i], MENU_BUTTON_SIZE, MENU_BUTTON_SIZE);
        lv_obj_align(demo->menu_buttons[i], LV_ALIGN_BOTTOM_RIGHT,
                     -(MENU_BUTTON_START_X - i * MENU_BUTTON_SPACING), 0);

        // Set default button style
        lv_obj_set_style_bg_color(demo->menu_buttons[i], lv_color_white(), 0);
        lv_obj_set_style_bg_opa(demo->menu_buttons[i], LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(demo->menu_buttons[i], 0, 0);
        lv_obj_set_style_radius(demo->menu_buttons[i], 3, 0);
        lv_obj_set_style_shadow_width(demo->menu_buttons[i], 0, 0);
        lv_obj_set_style_shadow_opa(demo->menu_buttons[i], LV_OPA_TRANSP, 0);

        // Custom icon buttons
        lv_obj_t *img = lv_img_create(demo->menu_buttons[i]);
        lv_img_set_src(img, menu_icons[i]);
        lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

        lv_obj_add_event_cb(demo->menu_buttons[i], menu_button_event_cb, LV_EVENT_CLICKED, demo);
    }

    // Highlight first button
    update_button_selection(0, 0);
}

/**
 * Creates the sub menu container
 */
static void create_sub_menu(ai_pet_demo_t *demo)
{
    demo->sub_menu = lv_obj_create(demo->screen);
    lv_obj_set_size(demo->sub_menu, AI_PET_SCREEN_WIDTH, AI_PET_SCREEN_HEIGHT);
    lv_obj_align(demo->sub_menu, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(demo->sub_menu, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(demo->sub_menu, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(demo->sub_menu, 0, 0);
    lv_obj_set_style_pad_all(demo->sub_menu, SUB_MENU_PADDING, 0);

    // Title at the top
    lv_obj_t *title = lv_label_create(demo->sub_menu);
    lv_label_set_text(title, "Menu");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, SUB_MENU_TITLE_OFFSET);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_black(), 0);

    // List for sub menu items
    demo->sub_menu_list = lv_list_create(demo->sub_menu);
    lv_obj_set_size(demo->sub_menu_list, AI_PET_SCREEN_WIDTH - 20, AI_PET_SCREEN_HEIGHT - 60);
    lv_obj_align(demo->sub_menu_list, LV_ALIGN_TOP_MID, 0, SUB_MENU_LIST_OFFSET);
    lv_obj_add_flag(demo->sub_menu_list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(demo->sub_menu_list, LV_DIR_VER);
    lv_obj_add_event_cb(demo->sub_menu_list, sub_menu_event_cb, LV_EVENT_CLICKED, demo);

    // Initially hide sub menu
    lv_obj_add_flag(demo->sub_menu, LV_OBJ_FLAG_HIDDEN);
}

/**
 * Pet animation callback - manages pet state and animations
 */
static void pet_animation_cb(lv_timer_t *timer)
{
    // The GIF animations are handled by the movement system
    // This callback can be used for additional pet state management

    // Ensure pet is always visible
    lv_obj_clear_flag(demo_data.current_pet_image, LV_OBJ_FLAG_HIDDEN);

    // Update pet state based on movement system
    if (demo_data.pet_is_walking) {
        demo_data.pet_state = AI_PET_STATE_WALKING;
    } else {
        demo_data.pet_state = AI_PET_STATE_IDLE;
    }
}

/**
 * Pet movement callback - natural movement with walking and idle states
 */
static void pet_movement_cb(lv_timer_t *timer)
{
    // Update state timer
    demo_data.pet_state_timer += PET_MOVEMENT_INTERVAL;

    // Check if it's time to change state
    if (demo_data.pet_state_timer >= demo_data.pet_state_duration) {
        // Switch between walking and idle
        demo_data.pet_is_walking = !demo_data.pet_is_walking;

                                                if (demo_data.pet_is_walking) {
            // Start walking - choose random direction and duration
            demo_data.pet_direction = (rand() % 2) ? 1 : -1;
            demo_data.pet_state_duration = PET_WALK_DURATION_MIN + (rand() % (PET_WALK_DURATION_MAX - PET_WALK_DURATION_MIN));



                        // Set appropriate animation based on direction
            if (demo_data.pet_direction == 1) {
                switch_pet_animation(demo_data.pet_image_walk);
            } else {
                switch_pet_animation(demo_data.pet_image_walk_left);
            }
        } else {
            // Start idle - choose random duration
            demo_data.pet_state_duration = PET_IDLE_DURATION_MIN + (rand() % (PET_IDLE_DURATION_MAX - PET_IDLE_DURATION_MIN));



                        // Use blink animation when idle
            switch_pet_animation(demo_data.pet_image_blink);
        }

        demo_data.pet_state_timer = 0;
    }

        // Move pet if walking
    if (demo_data.pet_is_walking) {
        demo_data.pet_x_pos += demo_data.pet_direction * PET_MOVEMENT_STEP;

        // Bounce off boundaries
        if (demo_data.pet_x_pos > PET_MOVEMENT_LIMIT) {
            demo_data.pet_x_pos = PET_MOVEMENT_LIMIT;
            demo_data.pet_direction = -1;
            switch_pet_animation(demo_data.pet_image_walk_left);
        } else if (demo_data.pet_x_pos < -PET_MOVEMENT_LIMIT) {
            demo_data.pet_x_pos = -PET_MOVEMENT_LIMIT;
            demo_data.pet_direction = 1;
            switch_pet_animation(demo_data.pet_image_walk);
        }
    } else {
        // Pet is idle - stays at current position
        // Animation is already set to blink in the state change logic
    }

    // Update pet position - move the container that holds the GIF widgets
    lv_obj_t *gif_container = lv_obj_get_parent(demo_data.current_pet_image);
    if (gif_container) {
        lv_obj_set_x(gif_container, demo_data.pet_x_pos);
    }


}



/**
 * Helper function to switch pet animations smoothly
 */
static void switch_pet_animation(lv_obj_t *new_animation)
{
    // Hide current animation
    lv_obj_add_flag(demo_data.current_pet_image, LV_OBJ_FLAG_HIDDEN);

    // Show new animation
    demo_data.current_pet_image = new_animation;
    lv_obj_clear_flag(new_animation, LV_OBJ_FLAG_HIDDEN);
}







/**
 * Menu button click event handler
 */
static void menu_button_event_cb(lv_event_t *e)
{
    ai_pet_demo_t *demo = (ai_pet_demo_t *)lv_event_get_user_data(e);
    lv_obj_t *btn = lv_event_get_target(e);

    // Find which button was clicked
    for(int i = 0; i < MENU_BUTTON_COUNT; i++) {
        if(demo->menu_buttons[i] == btn) {
            demo->selected_button = i;
            break;
        }
    }
}

/**
 * Sub menu item click event handler
 */
static void sub_menu_event_cb(lv_event_t *e)
{
    ai_pet_demo_t *demo = (ai_pet_demo_t *)lv_event_get_user_data(e);
    lv_obj_t *target = lv_event_get_target(e);

    printf("Sub menu item selected\n");

    if (demo->current_menu == AI_PET_MENU_INFO) {
        uint32_t child_count = lv_obj_get_child_cnt(demo->sub_menu_list);
        uint32_t action_items_start = find_action_items_start();

        // Find which action item was clicked
        for (uint32_t i = action_items_start; i < child_count; i++) {
            lv_obj_t *child = lv_obj_get_child(demo->sub_menu_list, i);
            if (child == target) {
                uint32_t action_index = i - action_items_start;
                if (action_index == 0) { // "Edit Pet Name" option
                    show_keyboard_for_pet_name(demo);
                }
                if (action_index == 1) {
                    lv_demo_ai_pocket_pet_show_toast("Not supported yet", 2000);
                }
                break;
            }
        }
    }
}

/**
 * Keyboard event handler
 */
static void keyboard_event_cb(lv_event_t *e)
{
    uint32_t key = lv_event_get_key(e);
    printf("Keyboard event received: key=%d\n", key);
    lv_demo_ai_pocket_pet_handle_input(key);
}

/**
 * Keyboard callback for pet name editing
 */
static void keyboard_callback(keyboard_result_t result, const char *text, void *user_data)
{
    ai_pet_demo_t *demo = (ai_pet_demo_t *)user_data;

    switch (result) {
        case KEYBOARD_RESULT_OK:
            if (text && strlen(text) > 0) {
                strncpy(demo->pet_stats.name, text, sizeof(demo->pet_stats.name) - 1);
                demo->pet_stats.name[sizeof(demo->pet_stats.name) - 1] = '\0';
                printf("Pet name updated to: %s\n", demo->pet_stats.name);

                if (demo->current_menu == AI_PET_MENU_INFO) {
                    show_info_menu(demo);
                }

                // Show toast message confirming name change
                lv_demo_ai_pocket_pet_show_toast("Pet name updated successfully!", 1500);
            }
            break;

        case KEYBOARD_RESULT_CANCEL:
            printf("Keyboard input cancelled\n");
            break;

        case KEYBOARD_RESULT_MENU:
            printf("Menu key pressed in keyboard\n");
            break;
    }

    // Reload the main screen to return from keyboard
    lv_screen_load(demo->screen);
    highlight_first_sub_menu_item(demo);
}

/**
 * Handles main menu navigation (up/down/left/right)
 */
static void handle_main_menu_navigation(uint32_t key)
{
    uint8_t old_selection = demo_data.selected_button;
    uint8_t new_selection = old_selection;

    switch(key) {
        case KEY_UP:
        case KEY_LEFT:
            if(demo_data.selected_button > 0) {
                new_selection = demo_data.selected_button - 1;
            }
            break;

        case KEY_DOWN:
        case KEY_RIGHT:
            if(demo_data.selected_button < MENU_BUTTON_COUNT - 1) {
                new_selection = demo_data.selected_button + 1;
            }
            break;
    }

    if(new_selection != old_selection) {
        demo_data.selected_button = new_selection;
        update_button_selection(old_selection, new_selection);
    }
}

/**
 * Handles sub menu navigation (up/down)
 */
static void handle_sub_menu_navigation(uint32_t key)
{
    uint32_t child_count = lv_obj_get_child_cnt(demo_data.sub_menu_list);
    if(child_count == 0) return;

    uint8_t old_selection = demo_data.sub_menu_selection;
    uint8_t new_selection = old_selection;

    switch(key) {
        case KEY_UP:
            if(demo_data.sub_menu_selection > 0) {
                new_selection = demo_data.sub_menu_selection - 1;
            }
            break;

        case KEY_DOWN:
            if(demo_data.sub_menu_selection < child_count - 1) {
                new_selection = demo_data.sub_menu_selection + 1;
            }
            break;
    }

    if(new_selection != old_selection) {
        update_sub_menu_selection(old_selection, new_selection);
        demo_data.sub_menu_selection = new_selection;
    }
}

/**
 * Handles main menu selection (ENTER key)
 */
static void handle_menu_selection(void)
{
    switch(demo_data.selected_button) {
        case 0: // Info
            show_info_menu(&demo_data);
            break;
        case 1: // Food
            show_food_menu(&demo_data);
            break;
        case 2: // Bath
            show_bath_menu(&demo_data);
            break;
        case 3: // Health
            show_health_menu(&demo_data);
            break;
        case 4: // Sleep
            show_sleep_menu(&demo_data);
            break;
        case 5: // Video
            show_video_menu(&demo_data);
            break;
    }
}

/**
 * Handles sub menu selection (ENTER key)
 */
static void handle_sub_menu_selection(void)
{
    if(demo_data.current_menu == AI_PET_MENU_INFO) {
        uint32_t action_items_start = find_action_items_start();
        uint32_t action_index = demo_data.sub_menu_selection - action_items_start;

        if (action_index == 0) {
            show_keyboard_for_pet_name(&demo_data);
        }
        if (action_index == 1) {
            lv_demo_ai_pocket_pet_show_toast("Not supported yet", 500);
        }
    }
}

/**
 * Handles AI function (I key)
 */
static void handle_ai_function(void)
{
    if(demo_data.current_menu == AI_PET_MENU_MAIN) {
        // Toggle pet state for demonstration
        demo_data.pet_state = (demo_data.pet_state + 1) % 5;
        printf("AI Function invoked! Pet state changed to: %d\n", demo_data.pet_state);

        // Update pet stats for testing
        demo_data.pet_stats.health = (demo_data.pet_stats.health + 5) % (MAX_STAT_VALUE + 1);
        demo_data.pet_stats.hungry = (demo_data.pet_stats.hungry + 10) % (MAX_STAT_VALUE + 1);
        demo_data.pet_stats.happy = (demo_data.pet_stats.happy + 3) % (MAX_STAT_VALUE + 1);
        demo_data.pet_stats.age_days++;
        demo_data.pet_stats.weight_kg += WEIGHT_INCREMENT;

        if(demo_data.pet_stats.weight_kg > MAX_WEIGHT_KG) {
            demo_data.pet_stats.weight_kg = MIN_WEIGHT_KG;
        }

        printf("Pet stats updated - Health: %d, Hungry: %d, Happy: %d, Age: %d days, Weight: %.1f kg\n",
               demo_data.pet_stats.health, demo_data.pet_stats.hungry, demo_data.pet_stats.happy,
               demo_data.pet_stats.age_days, demo_data.pet_stats.weight_kg);

        // Show toast message to confirm AI action
        lv_demo_ai_pocket_pet_show_toast("AI function activated! Pet stats updated.", 2000);
    }
}

/**
 * Shows the info menu with pet statistics
 */
/**
 * Creates the pet name display container
 */
static void create_pet_name_display(ai_pet_demo_t *demo)
{
    lv_obj_t *name_container = lv_obj_create(demo->sub_menu_list);
    lv_obj_set_size(name_container, STAT_CONTAINER_WIDTH, 40);
    lv_obj_set_style_bg_opa(name_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(name_container, 0, 0);
    lv_obj_set_style_pad_all(name_container, 2, 0);

    lv_obj_t *name_label = lv_label_create(name_container);
    lv_label_set_text_fmt(name_label, "Name: %s", demo->pet_stats.name);
    lv_obj_align(name_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(name_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(name_label, &lv_font_montserrat_14, 0);
}

/**
 * Creates all pet statistics displays
 */
static void create_pet_stats_displays(ai_pet_demo_t *demo)
{
    char value_str[16];

    snprintf(value_str, sizeof(value_str), "%d/100", demo->pet_stats.health);
    create_stat_display_item(demo->sub_menu_list, "Health:", value_str);

    snprintf(value_str, sizeof(value_str), "%d/100", demo->pet_stats.hungry);
    create_stat_display_item(demo->sub_menu_list, "Hungry:", value_str);

    snprintf(value_str, sizeof(value_str), "%d/100", demo->pet_stats.happy);
    create_stat_display_item(demo->sub_menu_list, "Happy:", value_str);

    snprintf(value_str, sizeof(value_str), "%d days", demo->pet_stats.age_days);
    create_stat_display_item(demo->sub_menu_list, "Age:", value_str);

    snprintf(value_str, sizeof(value_str), "%.1f kg", demo->pet_stats.weight_kg);
    create_stat_display_item(demo->sub_menu_list, "Weight:", value_str);
}

/**
 * Creates the separator line
 */
static void create_separator(void)
{
    lv_obj_t *separator = lv_obj_create(demo_data.sub_menu_list);
    lv_obj_set_size(separator, STAT_CONTAINER_WIDTH, SEPARATOR_HEIGHT);
    lv_obj_set_style_bg_color(separator, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(separator, LV_OPA_50, 0);
}

/**
 * Creates the actions section with title and buttons
 */
static void create_actions_section(void)
{
    // Add actions subtitle
    lv_obj_t *action_title = lv_label_create(demo_data.sub_menu_list);
    lv_label_set_text(action_title, "Actions:");
    lv_obj_align(action_title, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_set_style_text_color(action_title, lv_color_black(), 0);
    lv_obj_set_style_text_font(action_title, &lv_font_montserrat_14, 0);
    lv_obj_add_flag(action_title, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(action_title, LV_OBJ_FLAG_CLICK_FOCUSABLE);

    // Add action buttons
    lv_list_add_btn(demo_data.sub_menu_list, LV_SYMBOL_EDIT, "Edit Pet Name");
    lv_list_add_btn(demo_data.sub_menu_list, LV_SYMBOL_SETTINGS, "View Statistics");
    lv_list_add_btn(demo_data.sub_menu_list, LV_SYMBOL_EDIT, "WIFI Settings");
    lv_list_add_btn(demo_data.sub_menu_list, LV_SYMBOL_EDIT, "DEV:Randomize Pet Data");
}

/**
 * Shows the info menu with pet statistics and actions
 */
static void show_info_menu(ai_pet_demo_t *demo)
{
    demo->current_menu = AI_PET_MENU_INFO;
    lv_obj_clear_flag(demo->sub_menu, LV_OBJ_FLAG_HIDDEN);

    // Update title
    lv_obj_t *title = lv_obj_get_child(demo->sub_menu, 0);
    lv_label_set_text(title, "Pet Information");

    // Clear existing items
    lv_obj_clean(demo->sub_menu_list);

    // Create all UI components
    create_pet_name_display(demo);
    create_pet_stats_displays(demo);
    create_separator();
    create_actions_section();

    highlight_first_sub_menu_item(demo);
}

/**
 * Shows the food menu
 */
static void show_food_menu(ai_pet_demo_t *demo)
{
    const char *symbols[] = {LV_SYMBOL_EDIT, LV_SYMBOL_EDIT, LV_SYMBOL_EDIT, LV_SYMBOL_EDIT, LV_SYMBOL_EDIT};
    const char *items[] = {"Feed Hamberger", "Drink Water"};

    create_sub_menu_with_items(demo, "Food & Nutrition", symbols, items, 5);
}

/**
 * Shows the bath menu
 */
static void show_bath_menu(ai_pet_demo_t *demo)
{
    const char *symbols[] = {LV_SYMBOL_REFRESH, LV_SYMBOL_REFRESH, LV_SYMBOL_REFRESH, LV_SYMBOL_REFRESH, LV_SYMBOL_REFRESH};
    const char *items[] = {"Quick Wash", "Full Bath", "Brush Fur", "Spa Treatment", "Nail Trim"};

    create_sub_menu_with_items(demo, "Grooming & Care", symbols, items, 5);
}

/**
 * Shows the health menu
 */
static void show_health_menu(ai_pet_demo_t *demo)
{
    const char *symbols[] = {LV_SYMBOL_POWER, LV_SYMBOL_POWER, LV_SYMBOL_POWER, LV_SYMBOL_POWER, LV_SYMBOL_POWER};
    const char *items[] = {"Health Check", "Vaccination", "Give Medicine", "Exercise Time", "View Health Records"};

    create_sub_menu_with_items(demo, "Health & Wellness", symbols, items, 5);
}

/**
 * Shows the sleep menu
 */
static void show_sleep_menu(ai_pet_demo_t *demo)
{
    const char *symbols[] = {LV_SYMBOL_CLOSE, LV_SYMBOL_CLOSE, LV_SYMBOL_CLOSE, LV_SYMBOL_CLOSE, LV_SYMBOL_CLOSE};
    const char *items[] = {"Put to Sleep", "Wake Up Pet", "Set Bedtime", "Sleep Schedule", "Sleep Quality"};

    create_sub_menu_with_items(demo, "Sleep & Rest", symbols, items, 5);
}

/**
 * Shows the video menu (currently not implemented)
 */
static void show_video_menu(ai_pet_demo_t *demo)
{
    // TODO: Add video stream for multimodal feature
}

/**
 * Hides the sub menu and returns to main menu
 */
static void hide_sub_menu(ai_pet_demo_t *demo)
{
    demo->current_menu = AI_PET_MENU_MAIN;
    lv_obj_add_flag(demo->sub_menu, LV_OBJ_FLAG_HIDDEN);
}

/**
 * Shows keyboard for pet name editing
 */
static void show_keyboard_for_pet_name(ai_pet_demo_t *demo)
{
    lv_keyboard_widget_show(demo->pet_stats.name, keyboard_callback, demo);
}

/**
 * Updates button selection visual style
 */
static void update_button_selection(uint8_t old_selection, uint8_t new_selection)
{
    // Reset old button style
    lv_obj_set_style_bg_color(demo_data.menu_buttons[old_selection], lv_color_white(), 0);
    lv_obj_set_style_border_width(demo_data.menu_buttons[old_selection], 0, 0);
    lv_obj_set_style_shadow_width(demo_data.menu_buttons[old_selection], 0, 0);

    // Handle old button content styling
    lv_obj_t *old_child = lv_obj_get_child(demo_data.menu_buttons[old_selection], 0);
    if (old_child) {
        if (lv_obj_check_type(old_child, &lv_label_class)) {
            lv_obj_set_style_text_color(old_child, lv_color_black(), 0);
        } else if (lv_obj_check_type(old_child, &lv_image_class)) {
            // Reset image styling for unselected state
            lv_obj_set_style_img_recolor_opa(old_child, LV_OPA_TRANSP, 0);
            lv_obj_set_style_img_recolor(old_child, lv_color_black(), 0);
            lv_obj_set_style_img_opa(old_child, LV_OPA_COVER, 0);
        }
    }

    // Set new button style
    lv_obj_set_style_bg_color(demo_data.menu_buttons[new_selection], lv_color_black(), 0);
    lv_obj_set_style_border_color(demo_data.menu_buttons[new_selection], lv_color_black(), 0);
    lv_obj_set_style_border_width(demo_data.menu_buttons[new_selection], 2, 0);
    lv_obj_set_style_shadow_width(demo_data.menu_buttons[new_selection], 0, 0);

    // Handle new button content styling
    lv_obj_t *new_child = lv_obj_get_child(demo_data.menu_buttons[new_selection], 0);
    if (new_child) {
        if (lv_obj_check_type(new_child, &lv_label_class)) {
            lv_obj_set_style_text_color(new_child, lv_color_white(), 0);
        } else if (lv_obj_check_type(new_child, &lv_image_class)) {
            // Invert the image colors for selected state - black becomes white, white becomes black
            lv_obj_set_style_img_recolor_opa(new_child, LV_OPA_COVER, 0);
            lv_obj_set_style_img_recolor(new_child, lv_color_white(), 0);
            lv_obj_set_style_img_opa(new_child, LV_OPA_COVER, 0);
        }
    }
}

/**
 * Updates sub menu item selection visual style
 */
static void update_sub_menu_selection(uint8_t old_selection, uint8_t new_selection)
{
    uint32_t child_count = lv_obj_get_child_cnt(demo_data.sub_menu_list);

    if(old_selection < child_count) {
        lv_obj_set_style_bg_color(lv_obj_get_child(demo_data.sub_menu_list, old_selection), lv_color_white(), 0);
        lv_obj_set_style_text_color(lv_obj_get_child(demo_data.sub_menu_list, old_selection), lv_color_black(), 0);
    }

    if(new_selection < child_count) {
        lv_obj_set_style_bg_color(lv_obj_get_child(demo_data.sub_menu_list, new_selection), lv_color_black(), 0);
        lv_obj_set_style_text_color(lv_obj_get_child(demo_data.sub_menu_list, new_selection), lv_color_white(), 0);
        lv_obj_scroll_to_view(lv_obj_get_child(demo_data.sub_menu_list, new_selection), LV_ANIM_ON);
    }
}

/**
 * Initializes pet statistics with default values
 */
static void init_pet_stats(ai_pet_stats_t *stats)
{
    stats->health = 85;
    stats->hungry = 60;
    stats->happy = 90;
    stats->age_days = 15;
    stats->weight_kg = 1.2f;
    strcpy(stats->name, "Ducky");
}

/**
 * Updates pet stats display (currently only updates info menu if shown)
 */
static void update_pet_stats_display(ai_pet_demo_t *demo)
{
    if(demo->current_menu == AI_PET_MENU_INFO) {
        show_info_menu(demo);
    }
}

/**
 * Finds the starting index of action items in the sub menu
 */
static uint32_t find_action_items_start(void)
{
    uint32_t child_count = lv_obj_get_child_cnt(demo_data.sub_menu_list);

    for (uint32_t i = 0; i < child_count; i++) {
        lv_obj_t *child = lv_obj_get_child(demo_data.sub_menu_list, i);

        // Check if this child is a label (Actions: label)
        if (lv_obj_check_type(child, &lv_label_class)) {
            const char *text = lv_label_get_text(child);
            if (text && strstr(text, "Actions:") != NULL) {
                return i + 1;
            }
        }

        // Also check if this child has a label child (for containers)
        lv_obj_t *label = lv_obj_get_child(child, 0);
        if (label && lv_obj_check_type(label, &lv_label_class)) {
            const char *text = lv_label_get_text(label);
            if (text && strstr(text, "Actions:") != NULL) {
                return i + 1;
            }
        }
    }

    return 0;
}

/**
 * Creates a stat display item with label and value
 */
static void create_stat_display_item(lv_obj_t *parent, const char *label, const char *value)
{
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_set_size(container, STAT_CONTAINER_WIDTH, STAT_CONTAINER_HEIGHT);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 2, 0);

    lv_obj_t *label_obj = lv_label_create(container);
    lv_label_set_text(label_obj, label);
    lv_obj_align(label_obj, LV_ALIGN_LEFT_MID, 5, 0);
    lv_obj_set_style_text_color(label_obj, lv_color_black(), 0);

    lv_obj_t *value_obj = lv_label_create(container);
    lv_label_set_text(value_obj, value);
    lv_obj_align(value_obj, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_set_style_text_color(value_obj, lv_color_black(), 0);
}

/**
 * Highlights the first item in the sub menu
 */
static void highlight_first_sub_menu_item(ai_pet_demo_t *demo)
{
    demo->sub_menu_selection = 0;
    if(lv_obj_get_child_cnt(demo->sub_menu_list) > 0) {
        lv_obj_set_style_bg_color(lv_obj_get_child(demo->sub_menu_list, 0), lv_color_black(), 0);
        lv_obj_set_style_text_color(lv_obj_get_child(demo->sub_menu_list, 0), lv_color_white(), 0);
        lv_obj_scroll_to_view(lv_obj_get_child(demo->sub_menu_list, 0), LV_ANIM_ON);
    }
}

/**
 * Creates a sub menu with the given title and items
 */
static void create_sub_menu_with_items(ai_pet_demo_t *demo, const char *title,
                                     const char *symbols[], const char *items[], uint8_t item_count)
{
    demo->current_menu = AI_PET_MENU_INFO; // This will be overridden by specific menu functions
    lv_obj_clear_flag(demo->sub_menu, LV_OBJ_FLAG_HIDDEN);

    // Update title
    lv_obj_t *title_obj = lv_obj_get_child(demo->sub_menu, 0);
    lv_label_set_text(title_obj, title);

    lv_obj_clean(demo->sub_menu_list);

    // Add menu items
    for(uint8_t i = 0; i < item_count; i++) {
        lv_list_add_btn(demo->sub_menu_list, symbols[i], items[i]);
    }

    highlight_first_sub_menu_item(demo);
}

/**
 * Creates the toast message container and label
 */
static void create_toast_message(ai_pet_demo_t *demo)
{
    // Create toast container
    demo->toast_container = lv_obj_create(demo->screen);
    lv_obj_set_size(demo->toast_container, TOAST_MAX_WIDTH, TOAST_MIN_HEIGHT);
    lv_obj_align(demo->toast_container, LV_ALIGN_CENTER, 0, 0);

    // Style the toast container
    lv_obj_set_style_bg_color(demo->toast_container, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(demo->toast_container, LV_OPA_80, 0);
    lv_obj_set_style_border_width(demo->toast_container, 2, 0);
    lv_obj_set_style_border_color(demo->toast_container, lv_color_white(), 0);
    lv_obj_set_style_radius(demo->toast_container, 10, 0);
    lv_obj_set_style_pad_all(demo->toast_container, TOAST_PADDING, 0);
    lv_obj_set_style_shadow_width(demo->toast_container, 10, 0);
    lv_obj_set_style_shadow_color(demo->toast_container, lv_color_black(), 0);
    lv_obj_set_style_shadow_opa(demo->toast_container, LV_OPA_50, 0);

    // Create toast label
    demo->toast_label = lv_label_create(demo->toast_container);
    lv_label_set_text(demo->toast_label, "");
    lv_obj_align(demo->toast_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(demo->toast_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(demo->toast_label, &lv_font_montserrat_14, 0);
    lv_label_set_long_mode(demo->toast_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(demo->toast_label, TOAST_MAX_WIDTH - (TOAST_PADDING * 2));

    // Move toast to top of screen (highest z-order)
    lv_obj_move_foreground(demo->toast_container);

    // Initially hide the toast
    lv_obj_add_flag(demo->toast_container, LV_OBJ_FLAG_HIDDEN);

    // Initialize timer to NULL
    demo->toast_timer = NULL;
}

/**
 * Shows a toast message with the given text and delay
 */
static void show_toast_message(const char *message, uint32_t delay_ms)
{
    printf("show_toast_message called with: '%s'\n", message);

    // Hide any existing toast first
    hide_toast_message();

    // Set the message text
    lv_label_set_text(demo_data.toast_label, message);

    // Set a reasonable height for the toast container
    // The label will wrap text automatically within the container
    lv_obj_set_height(demo_data.toast_container, TOAST_MIN_HEIGHT);

    // Move toast to top of screen (highest z-order)
    lv_obj_move_foreground(demo_data.toast_container);

    // Show the toast immediately (without animation for testing)
    lv_obj_clear_flag(demo_data.toast_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_opa(demo_data.toast_container, LV_OPA_COVER, 0);
    printf("Toast container shown, opacity set to COVER\n");

    // Set up timer to hide the toast
    if (delay_ms > 0) {
        demo_data.toast_timer = lv_timer_create(toast_timer_cb, delay_ms, NULL);
    }
}

/**
 * Hides the toast message with animation
 */
static void hide_toast_message(void)
{
    printf("hide_toast_message called\n");

    // Cancel existing timer
    if (demo_data.toast_timer) {
        lv_timer_del(demo_data.toast_timer);
        demo_data.toast_timer = NULL;
    }

    // Hide immediately (without animation for testing)
    lv_obj_add_flag(demo_data.toast_container, LV_OBJ_FLAG_HIDDEN);
    printf("Toast container hidden\n");
}

/**
 * Toast timer callback - hides the toast when timer expires
 */
static void toast_timer_cb(lv_timer_t *timer)
{
    hide_toast_message();
}

/**
 * Toast animation ready callback - hides the toast container when animation completes
 */
static void toast_anim_ready_cb(lv_anim_t *a)
{
    lv_obj_add_flag(demo_data.toast_container, LV_OBJ_FLAG_HIDDEN);
}

/**
 * Public function to show a toast message
 * @param message The text message to display
 * @param delay_ms How long to show the toast (in milliseconds)
 */
void lv_demo_ai_pocket_pet_show_toast(const char *message, uint32_t delay_ms)
{
    printf("Showing toast: '%s' for %d ms\n", message, delay_ms);
    if (delay_ms == 0) {
        delay_ms = TOAST_DEFAULT_DELAY;
    }
    show_toast_message(message, delay_ms);
}

/**
 * Public function to hide the toast message immediately
 */
void lv_demo_ai_pocket_pet_hide_toast(void)
{
    hide_toast_message();
}

/**
 * Updates the WiFi icon based on signal strength
 * @param demo Pointer to the demo data
 * @param signal_strength 0 = off, 1-3 = bars, 4 = find, 5 = add
 */
static void update_wifi_icon(ai_pet_demo_t *demo, uint8_t signal_strength)
{
    const lv_img_dsc_t* icon = get_wifi_icon_by_strength(signal_strength);
    if (icon) {
        lv_img_set_src(demo->wifi_icon, icon);
    }
    demo->wifi_signal_strength = signal_strength;
}

/**
 * Updates the cellular icon based on signal strength and connection status
 * @param demo Pointer to the demo data
 * @param signal_strength 0 = off, 1-3 = bars, 4 = no internet
 * @param connected Whether cellular is connected to internet
 */
static void update_cellular_icon(ai_pet_demo_t *demo, uint8_t signal_strength, bool connected)
{
    const lv_img_dsc_t* icon = get_cellular_icon_by_strength(signal_strength, connected);
    if (icon) {
        lv_img_set_src(demo->network_icon, icon);
    }
    demo->cellular_signal_strength = signal_strength;
    demo->cellular_connected = connected;
}

/**
 * Updates the entire network status bar with current WiFi and cellular status
 * @param demo Pointer to the demo data
 */
static void update_network_status_bar(ai_pet_demo_t *demo)
{
    update_wifi_icon(demo, demo->wifi_signal_strength);
    update_cellular_icon(demo, demo->cellular_signal_strength, demo->cellular_connected);
}

/**
 * Returns the appropriate WiFi icon based on signal strength
 * @param strength 0 = off, 1-3 = bars, 4 = find, 5 = add
 * @return Pointer to the appropriate icon image descriptor
 */
static const lv_img_dsc_t* get_wifi_icon_by_strength(uint8_t strength)
{
    switch (strength) {
        case 0:
            return &wifi_off_icon;
        case 1:
            return &wifi_1_bar_icon;
        case 2:
            return &wifi_2_bar_icon;
        case 3:
            return &wifi_3_bar_icon;
        case 4:
            return &wifi_find_icon;
        case 5:
            return &wifi_add_icon;
        default:
            return &wifi_off_icon;
    }
}

/**
 * Returns the appropriate cellular icon based on signal strength and connection status
 * @param strength 0 = off, 1-3 = bars, 4 = no internet
 * @param connected Whether cellular is connected to internet
 * @return Pointer to the appropriate icon image descriptor
 */
static const lv_img_dsc_t* get_cellular_icon_by_strength(uint8_t strength, bool connected)
{
    if (strength == 0) {
        return &cellular_off_icon;
    }

    if (strength == 4 || !connected) {
        return &cellular_connected_no_internet_icon;
    }

    switch (strength) {
        case 1:
            return &cellular_1_bar_icon;
        case 2:
            return &cellular_2_bar_icon;
        case 3:
            return &cellular_3_bar_icon;
        default:
            return &cellular_off_icon;
    }
}

/**
 * Public function to set WiFi signal strength
 * @param strength 0 = off, 1-3 = bars, 4 = find, 5 = add
 */
void lv_demo_ai_pocket_pet_set_wifi_strength(uint8_t strength)
{
    if (strength <= 5) {
        update_wifi_icon(&demo_data, strength);
    }
}

/**
 * Public function to set cellular signal strength and connection status
 * @param strength 0 = off, 1-3 = bars, 4 = no internet
 * @param connected Whether cellular is connected to internet
 */
void lv_demo_ai_pocket_pet_set_cellular_status(uint8_t strength, bool connected)
{
    if (strength <= 4) {
        update_cellular_icon(&demo_data, strength, connected);
    }
}

/**
 * Public function to get current WiFi signal strength
 * @return Current WiFi signal strength (0-5)
 */
uint8_t lv_demo_ai_pocket_pet_get_wifi_strength(void)
{
    return demo_data.wifi_signal_strength;
}

/**
 * Public function to get current cellular signal strength
 * @return Current cellular signal strength (0-4)
 */
uint8_t lv_demo_ai_pocket_pet_get_cellular_strength(void)
{
    return demo_data.cellular_signal_strength;
}

/**
 * Public function to get current cellular connection status
 * @return Whether cellular is connected to internet
 */
bool lv_demo_ai_pocket_pet_get_cellular_connected(void)
{
    return demo_data.cellular_connected;
}

/**
 * Updates the battery icon based on level and charging status
 * @param demo Pointer to the demo data
 * @param level Battery level (0-6, where 0 = empty, 5 = 5 bars, 6 = full)
 * @param charging Whether battery is charging
 */
static void update_battery_icon(ai_pet_demo_t *demo, uint8_t level, bool charging)
{
    const lv_img_dsc_t* icon = get_battery_icon_by_level(level, charging);
    if (icon) {
        lv_img_set_src(demo->battery_icon, icon);
    }
    demo->battery_level = level;
    demo->battery_charging = charging;
}

/**
 * Returns the appropriate battery icon based on level and charging status
 * @param level Battery level (0-6, where 0 = empty, 5 = 5 bars, 6 = full)
 * @param charging Whether battery is charging
 * @return Pointer to the appropriate icon image descriptor
 */
static const lv_img_dsc_t* get_battery_icon_by_level(uint8_t level, bool charging)
{
    if (charging) {
        return &battery_charging_icon;
    }

    switch (level) {
        case 0:
            return &battery_0_icon;
        case 1:
            return &battery_1_icon;
        case 2:
            return &battery_2_icon;
        case 3:
            return &battery_3_icon;
        case 4:
            return &battery_4_icon;
        case 5:
            return &battery_5_icon;
        case 6:
            return &battery_full_icon;
        default:
            return &battery_full_icon;
    }
}

/**
 * Public function to set battery level and charging status
 * @param level Battery level (0-6, where 0 = empty, 5 = 5 bars, 6 = full)
 * @param charging Whether battery is charging
 */
void lv_demo_ai_pocket_pet_set_battery_status(uint8_t level, bool charging)
{
    if (level <= 6) {
        update_battery_icon(&demo_data, level, charging);
    }
}

/**
 * Public function to get current battery level
 * @return Current battery level (0-6)
 */
uint8_t lv_demo_ai_pocket_pet_get_battery_level(void)
{
    return demo_data.battery_level;
}

/**
 * Public function to get current battery charging status
 * @return Whether battery is charging
 */
bool lv_demo_ai_pocket_pet_get_battery_charging(void)
{
    return demo_data.battery_charging;
}
