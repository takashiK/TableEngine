#pragma once

/**
 * @file TeBinaryViewerSettings.h
 * @brief Declaration of TeBinaryViewer settings keys.
 * @ingroup viewer
 */

#ifdef _STTING_STR_DEF_
#define SETTING_STR(name, str) const char* name = str
#else
#define SETTING_STR(name, str) extern const char* name
#endif

SETTING_STR(SETTING_BINARY_VIEWER_WINDOW_WIDTH, "binary_viewer/layout/window_width");
SETTING_STR(SETTING_BINARY_VIEWER_WINDOW_HEIGHT, "binary_viewer/layout/window_height");
SETTING_STR(SETTING_BINARY_VIEWER_FONT_FAMILY, "binary_viewer/font/family");
SETTING_STR(SETTING_BINARY_VIEWER_FONT_SIZE, "binary_viewer/font/size");
SETTING_STR(SETTING_BINARY_VIEWER_DECODE_PANE_VISIBLE, "binary_viewer/decode/pane_visible");
SETTING_STR(SETTING_BINARY_VIEWER_DECODE_PANE_FLOATING, "binary_viewer/decode/pane_floating");
SETTING_STR(SETTING_BINARY_VIEWER_ENDIAN, "binary_viewer/decode/endian");

#undef SETTING_STR
