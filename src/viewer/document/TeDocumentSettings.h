#pragma once

/**
 * @file TeDocumentSettings.h
 * @brief Declaration of TeDocumentSettings.
 * @ingroup viewer
 */


#ifdef _STTING_STR_DEF_
#define SETTING_STR( name , str) const char* name = str
#else
#define SETTING_STR( name , str) extern const char* name
#endif

SETTING_STR(SETTING_TEXT_HIGHLIGHT_SCHEMA, "document_viewer/text_highlight_schema");
SETTING_STR(SETTING_TEXT_HIGHLIGHT_DEFAULT, "document_viewer/text_highlight_default");
SETTING_STR(SETTING_TEXT_HIGHLIGHT_FOLDER, "document_viewer/text_highlight_folder");
SETTING_STR(SETTING_TEXT_HIGHLIGHT_DIALOG_WIDTH, "document_viewer/text_highlight_dialog_width");
SETTING_STR(SETTING_TEXT_HIGHLIGHT_DIALOG_HEIGHT, "document_viewer/text_highlight_dialog_height");

SETTING_STR(SETTING_MARKDOWN_DEFAULT, "document_viewer/markdown_default");
SETTING_STR(SETTING_CONTAINER_FOLDER, "document_viewer/markdown_folder");

SETTING_STR(SETTING_TEXT_CODECS, "document_viewer/text_codecs");

SETTING_STR(SETTING_DOC_VIEWER_WINDOW_WIDTH, "document_viewer/layout/window_width");
SETTING_STR(SETTING_DOC_VIEWER_WINDOW_HEIGHT, "document_viewer/layout/window_height");

SETTING_STR(SETTING_DOC_VIEWER_WORD_WRAP, "document_viewer/text/word_wrap");
SETTING_STR(SETTING_DOC_VIEWER_LINE_NUMBER, "document_viewer/text/line_number");
SETTING_STR(SETTING_DOC_VIEWER_TAB_STOP, "document_viewer/text/tab_stop");

SETTING_STR(SETTING_DOC_VIEWER_FONT_FAMILY, "document_viewer/text/font_family");
SETTING_STR(SETTING_DOC_VIEWER_FONT_SIZE, "document_viewer/text/font_size");
SETTING_STR(SETTING_DOC_VIEWER_TEXT_CODEC, "document_viewer/text/codec");

#undef SETTING_STR
