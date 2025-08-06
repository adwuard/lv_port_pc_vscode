/**
 * @file lv_demo_ai_pocket_pet.c
 * AI Pocket Pet Demo for LVGL
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_demo_ai_pocket_pet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*********************
 *      DEFINES
 *********************/
#define STATUS_BAR_HEIGHT 20
#define BOTTOM_MENU_HEIGHT 40
#define PET_AREA_HEIGHT (AI_PET_SCREEN_HEIGHT - STATUS_BAR_HEIGHT - BOTTOM_MENU_HEIGHT)

// LVGL key codes
#define KEY_UP    17  // LV_KEY_UP
#define KEY_LEFT  20  // LV_KEY_LEFT
#define KEY_DOWN  18  // LV_KEY_DOWN
#define KEY_RIGHT 19  // LV_KEY_RIGHT
#define KEY_ENTER 10  // LV_KEY_ENTER
#define KEY_ESC   27  // LV_KEY_ESC
#define KEY_I     105 // 'i' key

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_obj_t *screen;
    lv_obj_t *status_bar;
    lv_obj_t *wifi_icon;
    lv_obj_t *network_icon;
    lv_obj_t *battery_icon;
    lv_obj_t *pet_area;
    lv_obj_t *pet_sprite;
    lv_obj_t *bottom_menu;
    lv_obj_t *menu_buttons[5];
    lv_obj_t *sub_menu;
    lv_obj_t *sub_menu_list;

    ai_pet_state_t pet_state;
    ai_pet_menu_t current_menu;
    uint8_t selected_button;
    uint8_t sub_menu_selection;

    lv_timer_t *pet_animation_timer;
    lv_timer_t *pet_movement_timer;
} ai_pet_demo_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void create_status_bar(ai_pet_demo_t *demo);
static void create_pet_area(ai_pet_demo_t *demo);
static void create_bottom_menu(ai_pet_demo_t *demo);
static void create_sub_menu(ai_pet_demo_t *demo);
static void pet_animation_cb(lv_timer_t *timer);
static void pet_movement_cb(lv_timer_t *timer);
static void menu_button_event_cb(lv_event_t *e);
static void sub_menu_event_cb(lv_event_t *e);
static void show_info_menu(ai_pet_demo_t *demo);
static void show_food_menu(ai_pet_demo_t *demo);
static void show_bath_menu(ai_pet_demo_t *demo);
static void show_health_menu(ai_pet_demo_t *demo);
static void show_sleep_menu(ai_pet_demo_t *demo);
static void hide_sub_menu(ai_pet_demo_t *demo);
static void keyboard_event_cb(lv_event_t *e);
static void update_button_selection(uint8_t old_selection, uint8_t new_selection);

/**********************
 *  STATIC VARIABLES
 **********************/
static ai_pet_demo_t demo_data;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_demo_ai_pocket_pet(void)
{
    // Initialize demo data
    memset(&demo_data, 0, sizeof(ai_pet_demo_t));
    demo_data.pet_state = AI_PET_STATE_IDLE;
    demo_data.current_menu = AI_PET_MENU_MAIN;
    demo_data.selected_button = 0;
    demo_data.sub_menu_selection = 0;

    // Create main screen
    demo_data.screen = lv_obj_create(NULL);
    lv_obj_set_size(demo_data.screen, AI_PET_SCREEN_WIDTH, AI_PET_SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(demo_data.screen, lv_color_white(), 0); // White background
    lv_obj_set_style_bg_opa(demo_data.screen, LV_OPA_COVER, 0);
    lv_screen_load(demo_data.screen);

    // Add keyboard event handler to the screen
    lv_obj_add_event_cb(demo_data.screen, keyboard_event_cb, LV_EVENT_KEY, NULL);

    // Make sure the screen can receive keyboard focus
    lv_group_add_obj(lv_group_get_default(), demo_data.screen);

    // Create UI components
    create_status_bar(&demo_data);
    create_pet_area(&demo_data);
    create_bottom_menu(&demo_data);
    create_sub_menu(&demo_data);

    // Start pet animation timer
    demo_data.pet_animation_timer = lv_timer_create(pet_animation_cb, 2000, &demo_data);
    demo_data.pet_movement_timer = lv_timer_create(pet_movement_cb, 3000, &demo_data);
}

void lv_demo_ai_pocket_pet_handle_input(uint32_t key)
{
    printf("Key pressed: %d (UP:%d LEFT:%d DOWN:%d RIGHT:%d ENTER:%d ESC:%d I:%d)\n",
           key, KEY_UP, KEY_LEFT, KEY_DOWN, KEY_RIGHT, KEY_ENTER, KEY_ESC, KEY_I);
    switch(key) {
                case KEY_UP: // Up
            printf("UP key pressed - navigating up\n");
            if(demo_data.current_menu == AI_PET_MENU_MAIN) {
                // Navigate bottom menu
                if(demo_data.selected_button > 0) {
                    uint8_t old_selection = demo_data.selected_button;
                    demo_data.selected_button--;
                    update_button_selection(old_selection, demo_data.selected_button);
                }
            } else {
                // Navigate sub menu
                if(demo_data.sub_menu_selection > 0) {
                    demo_data.sub_menu_selection--;
                    lv_obj_set_style_bg_color(lv_obj_get_child(demo_data.sub_menu_list, demo_data.sub_menu_selection), lv_color_black(), 0);
                    lv_obj_set_style_text_color(lv_obj_get_child(demo_data.sub_menu_list, demo_data.sub_menu_selection), lv_color_white(), 0);
                    if(demo_data.sub_menu_selection < lv_obj_get_child_cnt(demo_data.sub_menu_list) - 1) {
                        lv_obj_set_style_bg_color(lv_obj_get_child(demo_data.sub_menu_list, demo_data.sub_menu_selection + 1), lv_color_white(), 0);
                        lv_obj_set_style_text_color(lv_obj_get_child(demo_data.sub_menu_list, demo_data.sub_menu_selection + 1), lv_color_black(), 0);
                    }
                    // Scroll to the selected item
                    lv_obj_scroll_to_view(lv_obj_get_child(demo_data.sub_menu_list, demo_data.sub_menu_selection), LV_ANIM_ON);
                }
            }
            break;

                case KEY_DOWN: // Down
            printf("DOWN key pressed - navigating down\n");
            if(demo_data.current_menu == AI_PET_MENU_MAIN) {
                // Navigate bottom menu
                if(demo_data.selected_button < 4) {
                    uint8_t old_selection = demo_data.selected_button;
                    demo_data.selected_button++;
                    update_button_selection(old_selection, demo_data.selected_button);
                }
            } else {
                // Navigate sub menu
                if(demo_data.sub_menu_selection < lv_obj_get_child_cnt(demo_data.sub_menu_list) - 1) {
                    demo_data.sub_menu_selection++;
                    lv_obj_set_style_bg_color(lv_obj_get_child(demo_data.sub_menu_list, demo_data.sub_menu_selection), lv_color_black(), 0);
                    lv_obj_set_style_text_color(lv_obj_get_child(demo_data.sub_menu_list, demo_data.sub_menu_selection), lv_color_white(), 0);
                    if(demo_data.sub_menu_selection > 0) {
                        lv_obj_set_style_bg_color(lv_obj_get_child(demo_data.sub_menu_list, demo_data.sub_menu_selection - 1), lv_color_white(), 0);
                        lv_obj_set_style_text_color(lv_obj_get_child(demo_data.sub_menu_list, demo_data.sub_menu_selection - 1), lv_color_black(), 0);
                    }
                    // Scroll to the selected item
                    lv_obj_scroll_to_view(lv_obj_get_child(demo_data.sub_menu_list, demo_data.sub_menu_selection), LV_ANIM_ON);
                }
            }
            break;

                case KEY_LEFT: // Left
            printf("LEFT key pressed - navigating left\n");
            // Always navigate bottom menu regardless of current menu
            if(demo_data.selected_button > 0) {
                uint8_t old_selection = demo_data.selected_button;
                demo_data.selected_button--;
                update_button_selection(old_selection, demo_data.selected_button);
            }
            break;

                case KEY_RIGHT: // Right
            printf("RIGHT key pressed - navigating right\n");
            // Always navigate bottom menu regardless of current menu
            if(demo_data.selected_button < 4) {
                uint8_t old_selection = demo_data.selected_button;
                demo_data.selected_button++;
                update_button_selection(old_selection, demo_data.selected_button);
            }
            break;

        // Additional key code handling for different systems
        case 100: // 'd' key (alternative right)
        case 68:  // 'D' key (alternative right)
            printf("D key pressed - navigating right\n");
            if(demo_data.selected_button < 4) {
                uint8_t old_selection = demo_data.selected_button;
                demo_data.selected_button++;
                update_button_selection(old_selection, demo_data.selected_button);
            }
            break;

        case 97:  // 'a' key (alternative left)
        case 65:  // 'A' key (alternative left)
            printf("A key pressed - navigating left\n");
            if(demo_data.selected_button > 0) {
                uint8_t old_selection = demo_data.selected_button;
                demo_data.selected_button--;
                update_button_selection(old_selection, demo_data.selected_button);
            }
            break;

        case KEY_ENTER: // Select
            if(demo_data.current_menu == AI_PET_MENU_MAIN) {
                // Open sub menu based on selected button
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
                }
            }
            break;

        case KEY_ESC: // Back
            if(demo_data.current_menu != AI_PET_MENU_MAIN) {
                hide_sub_menu(&demo_data);
            }
            break;

        case KEY_I: // AI Function
            printf("I key pressed - AI function invoked\n");
            // Toggle pet state for demonstration
            demo_data.pet_state = (demo_data.pet_state + 1) % 5;
            printf("AI Function invoked! Pet state changed to: %d\n", demo_data.pet_state);
            break;
        default:
            printf("Unhandled key: %d\n", key);
            // Test if any key press is working
            if(key > 0) {
                printf("Key press detected but not handled: %d\n", key);
            }
            break;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void create_status_bar(ai_pet_demo_t *demo)
{
    demo->status_bar = lv_obj_create(demo->screen);
    lv_obj_set_size(demo->status_bar, AI_PET_SCREEN_WIDTH, STATUS_BAR_HEIGHT);
    lv_obj_align(demo->status_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(demo->status_bar, LV_OPA_TRANSP, 0); // Transparent background
    lv_obj_set_style_border_width(demo->status_bar, 0, 0);
    lv_obj_set_style_pad_all(demo->status_bar, 2, 0);

        // WiFi icon
    demo->wifi_icon = lv_label_create(demo->status_bar);
    lv_label_set_text(demo->wifi_icon, LV_SYMBOL_WIFI);
    lv_obj_align(demo->wifi_icon, LV_ALIGN_LEFT_MID, 5, 0);
    lv_obj_set_style_text_color(demo->wifi_icon, lv_color_black(), 0);

    // Network icon (4G)
    demo->network_icon = lv_label_create(demo->status_bar);
    lv_label_set_text(demo->network_icon, "4G");
    lv_obj_align(demo->network_icon, LV_ALIGN_LEFT_MID, 30, 0);
    lv_obj_set_style_text_color(demo->network_icon, lv_color_black(), 0);

    // Battery icon
    demo->battery_icon = lv_label_create(demo->status_bar);
    lv_label_set_text(demo->battery_icon, LV_SYMBOL_BATTERY_FULL);
    lv_obj_align(demo->battery_icon, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_set_style_text_color(demo->battery_icon, lv_color_black(), 0);
}

static void create_pet_area(ai_pet_demo_t *demo)
{
    demo->pet_area = lv_obj_create(demo->screen);
    lv_obj_set_size(demo->pet_area, AI_PET_SCREEN_WIDTH, PET_AREA_HEIGHT);
    lv_obj_align(demo->pet_area, LV_ALIGN_TOP_MID, 0, STATUS_BAR_HEIGHT);
    lv_obj_set_style_bg_opa(demo->pet_area, LV_OPA_TRANSP, 0); // Transparent background
    lv_obj_set_style_border_width(demo->pet_area, 0, 0);
    lv_obj_set_style_pad_all(demo->pet_area, 5, 0);

    // Create pet sprite (placeholder)
    demo->pet_sprite = lv_obj_create(demo->pet_area);
    lv_obj_set_size(demo->pet_sprite, 40, 40);
    lv_obj_align(demo->pet_sprite, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(demo->pet_sprite, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(demo->pet_sprite, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(demo->pet_sprite, 20, 0);

    // Add pet label
    lv_obj_t *pet_label = lv_label_create(demo->pet_sprite);
    lv_label_set_text(pet_label, "🐾");
    lv_obj_align(pet_label, LV_ALIGN_CENTER, 0, 0);
}

static void create_bottom_menu(ai_pet_demo_t *demo)
{
    demo->bottom_menu = lv_obj_create(demo->screen);
    lv_obj_set_size(demo->bottom_menu, AI_PET_SCREEN_WIDTH, BOTTOM_MENU_HEIGHT);
    lv_obj_align(demo->bottom_menu, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(demo->bottom_menu, LV_OPA_TRANSP, 0); // Transparent background
    lv_obj_set_style_border_width(demo->bottom_menu, 0, 0);
    lv_obj_set_style_pad_all(demo->bottom_menu, 5, 0);

    const char *menu_symbols[] = {LV_SYMBOL_SETTINGS, LV_SYMBOL_EDIT, LV_SYMBOL_REFRESH, LV_SYMBOL_POWER, LV_SYMBOL_CLOSE};

        // Position icons closer to bottom right
    int start_x = AI_PET_SCREEN_WIDTH - 260; // Start closer to right edge
    int icon_spacing = 30; // 26px border + 4px spacing

    for(int i = 0; i < 5; i++) {
        demo->menu_buttons[i] = lv_btn_create(demo->bottom_menu);
        lv_obj_set_size(demo->menu_buttons[i], 24, 24); // 24x24 icon size
        lv_obj_align(demo->menu_buttons[i], LV_ALIGN_BOTTOM_RIGHT, -(start_x - i * icon_spacing), -5);
        lv_obj_set_style_bg_color(demo->menu_buttons[i], lv_color_white(), 0);
        lv_obj_set_style_bg_opa(demo->menu_buttons[i], LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(demo->menu_buttons[i], 0, 0); // No border by default
        lv_obj_set_style_radius(demo->menu_buttons[i], 3, 0); // Slightly larger radius for 24x24 icons
        lv_obj_set_style_shadow_width(demo->menu_buttons[i], 0, 0); // No shadow
        lv_obj_set_style_shadow_opa(demo->menu_buttons[i], LV_OPA_TRANSP, 0); // Transparent shadow

        lv_obj_t *label = lv_label_create(demo->menu_buttons[i]);
        lv_label_set_text(label, menu_symbols[i]); // Use symbols instead of text
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_color(label, lv_color_black(), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);

        lv_obj_add_event_cb(demo->menu_buttons[i], menu_button_event_cb, LV_EVENT_CLICKED, demo);
    }

    // Highlight first button with border
    lv_obj_set_style_bg_color(demo->menu_buttons[0], lv_color_black(), 0);
    lv_obj_set_style_border_color(demo->menu_buttons[0], lv_color_black(), 0);
    lv_obj_set_style_border_width(demo->menu_buttons[0], 2, 0); // 2px border for selected (26x26 total)
    lv_obj_set_style_text_color(lv_obj_get_child(demo->menu_buttons[0], 0), lv_color_white(), 0);
    lv_obj_set_style_shadow_width(demo->menu_buttons[0], 0, 0); // No shadow for selected too
}

static void create_sub_menu(ai_pet_demo_t *demo)
{
    demo->sub_menu = lv_obj_create(demo->screen);
    lv_obj_set_size(demo->sub_menu, AI_PET_SCREEN_WIDTH, AI_PET_SCREEN_HEIGHT);
    lv_obj_align(demo->sub_menu, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(demo->sub_menu, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(demo->sub_menu, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(demo->sub_menu, 0, 0);
    lv_obj_set_style_pad_all(demo->sub_menu, 10, 0);

    // Title at the top
    lv_obj_t *title = lv_label_create(demo->sub_menu);
    lv_label_set_text(title, "Menu");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_black(), 0);

        // List for sub menu items - fill remaining space
    demo->sub_menu_list = lv_list_create(demo->sub_menu);
    lv_obj_set_size(demo->sub_menu_list, AI_PET_SCREEN_WIDTH - 20, AI_PET_SCREEN_HEIGHT - 60);
    lv_obj_align(demo->sub_menu_list, LV_ALIGN_TOP_MID, 0, 40);

    // Enable scroll UI for keyboard navigation
    lv_obj_add_flag(demo->sub_menu_list, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_event_cb(demo->sub_menu_list, sub_menu_event_cb, LV_EVENT_CLICKED, demo);

    // Initially hide sub menu
    lv_obj_add_flag(demo->sub_menu, LV_OBJ_FLAG_HIDDEN);
}

static void pet_animation_cb(lv_timer_t *timer)
{
    // Simple animation - change pet color based on state
    lv_color_t colors[] = {
        lv_color_black(), // Idle - Black
        lv_color_black(), // Walking - Black
        lv_color_black(), // Eating - Black
        lv_color_black(), // Sleeping - Black
        lv_color_black()  // Playing - Black
    };

    lv_obj_set_style_bg_color(demo_data.pet_sprite, colors[demo_data.pet_state], 0);
}

static void pet_movement_cb(lv_timer_t *timer)
{
    // Simple random movement
    static int8_t direction = 1;
    static int16_t x_pos = 0;

    x_pos += direction * 10;
    if(x_pos > 150 || x_pos < -150) {
        direction *= -1;
    }

    lv_obj_set_x(demo_data.pet_sprite, x_pos);
}

static void menu_button_event_cb(lv_event_t *e)
{
    ai_pet_demo_t *demo = (ai_pet_demo_t *)lv_event_get_user_data(e);
    lv_obj_t *btn = lv_event_get_target(e);

    // Find which button was clicked
    for(int i = 0; i < 5; i++) {
        if(demo->menu_buttons[i] == btn) {
            demo->selected_button = i;
            break;
        }
    }
}

static void sub_menu_event_cb(lv_event_t *e)
{
    ai_pet_demo_t *demo = (ai_pet_demo_t *)lv_event_get_user_data(e);
    // Handle sub menu item selection
    printf("Sub menu item selected\n");
}

static void show_info_menu(ai_pet_demo_t *demo)
{
    demo->current_menu = AI_PET_MENU_INFO;
    lv_obj_clear_flag(demo->sub_menu, LV_OBJ_FLAG_HIDDEN);

    // Update title
    lv_obj_t *title = lv_obj_get_child(demo->sub_menu, 0);
    lv_label_set_text(title, "Pet Information");

    // Clear existing items
    lv_obj_clean(demo->sub_menu_list);

    // Add actionable info items
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_FILE, "View Pet Profile");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_EDIT, "Edit Pet Name");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_SETTINGS, "View Statistics");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_SETTINGS, "Check Status");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_EDIT, "View History");

    // Highlight first item
    demo->sub_menu_selection = 0;
    if(lv_obj_get_child_cnt(demo->sub_menu_list) > 0) {
        lv_obj_set_style_bg_color(lv_obj_get_child(demo->sub_menu_list, 0), lv_color_black(), 0);
        lv_obj_set_style_text_color(lv_obj_get_child(demo->sub_menu_list, 0), lv_color_white(), 0);
        // Scroll to the first item
        lv_obj_scroll_to_view(lv_obj_get_child(demo->sub_menu_list, 0), LV_ANIM_ON);
    }
}

static void show_food_menu(ai_pet_demo_t *demo)
{
    demo->current_menu = AI_PET_MENU_FOOD;
    lv_obj_clear_flag(demo->sub_menu, LV_OBJ_FLAG_HIDDEN);

    // Update title
    lv_obj_t *title = lv_obj_get_child(demo->sub_menu, 0);
    lv_label_set_text(title, "Food & Nutrition");

    lv_obj_clean(demo->sub_menu_list);

    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_EDIT, "Feed Dry Food");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_EDIT, "Feed Wet Food");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_EDIT, "Give Treats");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_EDIT, "Special Meal");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_EDIT, "Set Feeding Schedule");

    demo->sub_menu_selection = 0;
    if(lv_obj_get_child_cnt(demo->sub_menu_list) > 0) {
        lv_obj_set_style_bg_color(lv_obj_get_child(demo->sub_menu_list, 0), lv_color_black(), 0);
        lv_obj_set_style_text_color(lv_obj_get_child(demo->sub_menu_list, 0), lv_color_white(), 0);
        // Scroll to the first item
        lv_obj_scroll_to_view(lv_obj_get_child(demo->sub_menu_list, 0), LV_ANIM_ON);
    }
}

static void show_bath_menu(ai_pet_demo_t *demo)
{
    demo->current_menu = AI_PET_MENU_BATH;
    lv_obj_clear_flag(demo->sub_menu, LV_OBJ_FLAG_HIDDEN);

    // Update title
    lv_obj_t *title = lv_obj_get_child(demo->sub_menu, 0);
    lv_label_set_text(title, "Grooming & Care");

    lv_obj_clean(demo->sub_menu_list);

    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_REFRESH, "Quick Wash");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_REFRESH, "Full Bath");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_REFRESH, "Brush Fur");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_REFRESH, "Spa Treatment");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_REFRESH, "Nail Trim");

    demo->sub_menu_selection = 0;
    if(lv_obj_get_child_cnt(demo->sub_menu_list) > 0) {
        lv_obj_set_style_bg_color(lv_obj_get_child(demo->sub_menu_list, 0), lv_color_black(), 0);
        lv_obj_set_style_text_color(lv_obj_get_child(demo->sub_menu_list, 0), lv_color_white(), 0);
        // Scroll to the first item
        lv_obj_scroll_to_view(lv_obj_get_child(demo->sub_menu_list, 0), LV_ANIM_ON);
    }
}

static void show_health_menu(ai_pet_demo_t *demo)
{
    demo->current_menu = AI_PET_MENU_HEALTH;
    lv_obj_clear_flag(demo->sub_menu, LV_OBJ_FLAG_HIDDEN);

    // Update title
    lv_obj_t *title = lv_obj_get_child(demo->sub_menu, 0);
    lv_label_set_text(title, "Health & Wellness");

    lv_obj_clean(demo->sub_menu_list);

    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_POWER, "Health Check");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_POWER, "Vaccination");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_POWER, "Give Medicine");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_POWER, "Exercise Time");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_POWER, "View Health Records");

    demo->sub_menu_selection = 0;
    if(lv_obj_get_child_cnt(demo->sub_menu_list) > 0) {
        lv_obj_set_style_bg_color(lv_obj_get_child(demo->sub_menu_list, 0), lv_color_black(), 0);
        lv_obj_set_style_text_color(lv_obj_get_child(demo->sub_menu_list, 0), lv_color_white(), 0);
        // Scroll to the first item
        lv_obj_scroll_to_view(lv_obj_get_child(demo->sub_menu_list, 0), LV_ANIM_ON);
    }
}

static void show_sleep_menu(ai_pet_demo_t *demo)
{
    demo->current_menu = AI_PET_MENU_SLEEP;
    lv_obj_clear_flag(demo->sub_menu, LV_OBJ_FLAG_HIDDEN);

    // Update title
    lv_obj_t *title = lv_obj_get_child(demo->sub_menu, 0);
    lv_label_set_text(title, "Sleep & Rest");

    lv_obj_clean(demo->sub_menu_list);

    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_CLOSE, "Put to Sleep");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_CLOSE, "Wake Up Pet");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_CLOSE, "Set Bedtime");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_CLOSE, "Sleep Schedule");
    lv_list_add_btn(demo->sub_menu_list, LV_SYMBOL_CLOSE, "Sleep Quality");

    demo->sub_menu_selection = 0;
    if(lv_obj_get_child_cnt(demo->sub_menu_list) > 0) {
        lv_obj_set_style_bg_color(lv_obj_get_child(demo->sub_menu_list, 0), lv_color_black(), 0);
        lv_obj_set_style_text_color(lv_obj_get_child(demo->sub_menu_list, 0), lv_color_white(), 0);
        // Scroll to the first item
        lv_obj_scroll_to_view(lv_obj_get_child(demo->sub_menu_list, 0), LV_ANIM_ON);
    }
}

static void hide_sub_menu(ai_pet_demo_t *demo)
{
    demo->current_menu = AI_PET_MENU_MAIN;
    lv_obj_add_flag(demo->sub_menu, LV_OBJ_FLAG_HIDDEN);
}

static void update_button_selection(uint8_t old_selection, uint8_t new_selection)
{
    // Reset old button style - no border, no shadow
    lv_obj_set_style_bg_color(demo_data.menu_buttons[old_selection], lv_color_white(), 0);
    lv_obj_set_style_border_width(demo_data.menu_buttons[old_selection], 0, 0); // No border
    lv_obj_set_style_text_color(lv_obj_get_child(demo_data.menu_buttons[old_selection], 0), lv_color_black(), 0);
    lv_obj_set_style_shadow_width(demo_data.menu_buttons[old_selection], 0, 0); // No shadow

    // Set new button style - with border, no shadow
    lv_obj_set_style_bg_color(demo_data.menu_buttons[new_selection], lv_color_black(), 0);
    lv_obj_set_style_border_color(demo_data.menu_buttons[new_selection], lv_color_black(), 0);
    lv_obj_set_style_border_width(demo_data.menu_buttons[new_selection], 2, 0); // 2px border (26x26 total)
    lv_obj_set_style_text_color(lv_obj_get_child(demo_data.menu_buttons[new_selection], 0), lv_color_white(), 0);
    lv_obj_set_style_shadow_width(demo_data.menu_buttons[new_selection], 0, 0); // No shadow
}

static void keyboard_event_cb(lv_event_t *e)
{
    uint32_t key = lv_event_get_key(e);
    printf("Keyboard event received: key=%d\n", key);
    lv_demo_ai_pocket_pet_handle_input(key);
}
