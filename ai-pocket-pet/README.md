# AI Pocket Pet Demo

A Tamagotchi-like UI demo for LVGL with a virtual pet interface.

## Features

- **Status Bar**: Shows WiFi, 4G network, and battery status
- **Pet Area**: Central area where the pet walks and animates
- **Bottom Menu**: 5 interactive buttons (Info, Food, Bath, Health, Sleep)
- **Sub Menus**: Detailed options for each category
- **Keyboard Input**: WASD navigation, N/M for select/back, I for AI function

## Controls

- **W/A/S/D**: Navigate menu items
- **N**: Select/Enter menu
- **M**: Back/Return to previous menu
- **I**: Invoke AI function (changes pet state)

## Screen Layout

- **384x168** resolution
- **Status Bar** (top 20px): WiFi, 4G, Battery icons
- **Pet Area** (middle): Animated pet sprite
- **Bottom Menu** (bottom 40px): 5 action buttons

## Menu Structure

### Main Menu
- **Info**: Pet information display
- **Food**: Food selection menu
- **Bath**: Bath and grooming options
- **Health**: Health care options
- **Sleep**: Sleep management

### Sub Menus
Each sub menu contains relevant options with placeholder data for demonstration.

## Pet States

The pet has 5 different states that change color:
- **Idle** (Orange)
- **Walking** (Green)
- **Eating** (Orange)
- **Sleeping** (Blue)
- **Playing** (Purple)

## Building and Running

1. Ensure the demo is enabled in `lv_conf.h`
2. Build the project: `cmake --build build`
3. Run the executable: `./bin/main`
4. Use keyboard controls to interact with the demo

## Customization

- Modify `AI_PET_SCREEN_WIDTH` and `AI_PET_SCREEN_HEIGHT` for different screen sizes
- Add new pet states in `ai_pet_state_t` enum
- Extend menu options in the sub menu functions
- Replace placeholder pet sprite with actual graphics
