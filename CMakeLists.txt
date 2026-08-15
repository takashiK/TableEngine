cmake_minimum_required(VERSION 3.19)

project(TableEngine VERSION 0.2.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets LinguistTools)

qt_standard_project_setup(
	I18N_SOURCE_LANGUAGE en
	I18N_TRANSLATED_LANGUAGES ja
)

enable_testing()

add_subdirectory(main)
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(viewer_tests)
add_subdirectory(support/QHexView)
add_subdirectory(support/QLightTerminal)

qt_add_translations(TableEngine
	SOURCE_TARGETS TableEngine tablelibs
	TS_FILES ${CMAKE_SOURCE_DIR}/translations/tableengine_ja.ts
	RESOURCE_PREFIX /i18n
)

# --- CPack packaging ---
set(CPACK_PACKAGE_NAME              "TableEngine")
set(CPACK_PACKAGE_VENDOR            "Takashi Kuwabara")
set(CPACK_PACKAGE_VERSION           "${PROJECT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME         "TableEngine-${PROJECT_VERSION}-win64")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TableEngine file manager")
set(CPACK_RESOURCE_FILE_LICENSE     "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_GENERATOR                 "ZIP")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)
