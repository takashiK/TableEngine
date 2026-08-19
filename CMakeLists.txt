cmake_minimum_required(VERSION 3.19)

project(TableEngine VERSION 0.2.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets LinguistTools)

include(GNUInstallDirs)

# I18N_SOURCE_LANGUAGE / I18N_TRANSLATED_LANGUAGES require Qt 6.7+; on Qt 6.4
# (Ubuntu 24.04 apt package) qt_standard_project_setup() takes no arguments.
if(Qt6_VERSION VERSION_GREATER_EQUAL "6.7")
    qt_standard_project_setup(
        I18N_SOURCE_LANGUAGE en
        I18N_TRANSLATED_LANGUAGES ja
    )
else()
    qt_standard_project_setup()
endif()

enable_testing()

add_subdirectory(main)
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(viewer_tests)
add_subdirectory(support/QHexView)
add_subdirectory(support/QLightTerminal)

# SOURCE_TARGETS requires Qt 6.7+. On Qt 6.4 (Ubuntu 24.04 apt package),
# qt_add_translations() without SOURCE_TARGETS would still create an
# "update_translations" target, but lupdate would scan only the TableEngine
# target sources (missing tablelibs), silently corrupting the .ts on refresh.
# Since translations/tableengine_ja.ts is maintained manually, just compile the
# existing .ts into an embedded .qm resource via the long-standing
# qt_add_translation() (singular) command instead of creating that target.
if(Qt6_VERSION VERSION_GREATER_EQUAL "6.7")
    qt_add_translations(TableEngine
        SOURCE_TARGETS TableEngine tablelibs
        TS_FILES ${CMAKE_SOURCE_DIR}/translations/tableengine_ja.ts
        RESOURCE_PREFIX /i18n
    )
else()
    qt_add_translation(QM_FILES ${CMAKE_SOURCE_DIR}/translations/tableengine_ja.ts)
    # qt_add_translation() places the .qm next to this CMakeLists.txt's
    # binary dir; BASE must match so the resource alias resolves to plain
    # "tableengine_ja.qm" and main.cpp's ":/i18n/tableengine_ja.qm" load works.
    qt6_add_resources(TableEngine "tableengine_translations"
        PREFIX /i18n
        BASE ${CMAKE_CURRENT_BINARY_DIR}
        FILES ${QM_FILES}
    )
endif()

# --- CPack packaging ---
set(CPACK_PACKAGE_NAME              "TableEngine")
set(CPACK_PACKAGE_VENDOR            "Takashi Kuwabara")
set(CPACK_PACKAGE_VERSION           "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TableEngine file manager")
set(CPACK_RESOURCE_FILE_LICENSE     "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

if(WIN32)
    set(CPACK_PACKAGE_FILE_NAME "TableEngine-${PROJECT_VERSION}-win64")
    set(CPACK_GENERATOR         "ZIP")
else()
    set(CPACK_PACKAGE_FILE_NAME "tableengine-${PROJECT_VERSION}-linux-amd64")
    set(CPACK_GENERATOR         "DEB")
    # Force /usr regardless of the dev CMAKE_INSTALL_PREFIX used to configure
    # the build tree, so the DEB always installs to the standard FHS location.
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
    set(CPACK_DEBIAN_PACKAGE_NAME       "tableengine")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Takashi Kuwabara <laffile@gmail.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION    "utils")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS  ON)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS    "qt6-qpa-plugins")
endif()

include(CPack)
