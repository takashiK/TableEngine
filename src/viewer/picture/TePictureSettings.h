#pragma once

/**
 * @file TePictureSettings.h
 * @brief Declaration of TePictureSettings.
 * @ingroup viewer
 */


#ifdef _SETTING_STR_DEF_
#define SETTING_STR( name , str) const char* name = str
#else
#define SETTING_STR( name , str) extern const char* name
#endif

SETTING_STR(SETTING_PIC_VIEWER_WINDOW_WIDTH, "picture_viewer/layout/window_width");
SETTING_STR(SETTING_PIC_VIEWER_WINDOW_HEIGHT, "picture_viewer/layout/window_height");

#undef SETTING_STR
