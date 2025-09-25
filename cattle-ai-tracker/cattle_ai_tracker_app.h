/**
 * @file cattle_ai_tracker_app.h
 */

#ifndef CATTLE_AI_TRACKER_APP_H
#define CATTLE_AI_TRACKER_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../lvgl/lvgl.h"

#ifndef CATTLE_SCREEN_WIDTH
#define CATTLE_SCREEN_WIDTH 466
#endif
#ifndef CATTLE_SCREEN_HEIGHT
#define CATTLE_SCREEN_HEIGHT 466
#endif

void lv_demo_cattle_ai_tracker(void);
void update_idle_bottom_text(const char* text);

#ifdef __cplusplus
}
#endif

#endif /* CATTLE_AI_TRACKER_APP_H */
