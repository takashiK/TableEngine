#pragma once

#ifdef _STTING_STR_DEF_
#define SETTING_STR( name , str) const char* name = str
#else
#define SETTING_STR( name , str) extern const char* name
#endif

SETTING_STR(SETTING_TEXT_HIGHLIGHT_SCHEMA, "document_viewer/text_highlight_schema");
SETTING_STR(SETTING_TEXT_HIGHLIGHT_FOLDER, "document_viewer/text_highlight_folder");

#undef SETTING_STR
