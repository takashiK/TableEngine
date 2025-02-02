# CMakeLists.txt for tablelibs
file(GLOB SOURCES_BASE             RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
file(GLOB SOURCES_PLATFORM         RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "platform/*.cpp")
file(GLOB_RECURSE SOURCES_COMMANDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "commands/*.cpp")
file(GLOB_RECURSE SOURCES_VIEW_DOC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "viewer/document/*.cpp")
file(GLOB_RECURSE SOURCES_VIEW_PIC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "viewer/picture/*.cpp")
file(GLOB_RECURSE SOURCES_VIEW_BIN RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "viewer/binary/*.cpp")
file(GLOB_RECURSE SOURCES_DIALOGS  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "dialogs/*.cpp")
file(GLOB_RECURSE SOURCES_WIDGETS  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "widgets/*.cpp")

file(GLOB HEADERS_BASE             RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
file(GLOB HEADERS_PLATFORM         RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "platform/*.h")
file(GLOB_RECURSE HEADERS_COMMANDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "commands/*.h")
file(GLOB_RECURSE HEADERS_VIEW_DOC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "viewer/document/*.h")
file(GLOB_RECURSE HEADERS_VIEW_PIC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "viewer/picture/*.h")
file(GLOB_RECURSE HEADERS_VIEW_BIN RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "viewer/binary/*.h")
file(GLOB_RECURSE HEADERS_DIALOGS  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "dialogs/*.h")
file(GLOB_RECURSE HEADERS_WIDGETS  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "widgets/*.h")


# Windows platform specific files
file(GLOB_RECURSE SOURCES_WIN RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "platform/windows/*.cpp")
file(GLOB_RECURSE HEADERS_WIN RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "platform/windows/*.h")

# Support package include directory
set(SUPPORT_INCLUDE_DIR )
set(SUPPORT_LIBS )

# Add platform specific files
if(WIN32)
    list(APPEND SOURCES_PLATFORM ${SOURCES_WIN})    
    list(APPEND HEADERS_PLATFORM ${HEADERS_WIN})    

    list(APPEND SUPPORT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../support_package/include)
    
endif(WIN32)

qt_add_library(tablelibs STATIC
    ${SOURCES_BASE}
    ${SOURCES_COMMANDS}
    ${SOURCES_VIEW_DOC}
    ${SOURCES_VIEW_PIC}
    ${SOURCES_VIEW_BIN}
    ${SOURCES_DIALOGS}
    ${SOURCES_PLATFORM}
    ${SOURCES_WIDGETS}

    ${HEADERS_BASE}
    ${HEADERS_COMMANDS}
    ${HEADERS_VIEW_DOC}
    ${HEADERS_VIEW_PIC}
    ${HEADERS_VIEW_BIN}
    ${HEADERS_DIALOGS}
    ${HEADERS_PLATFORM}
    ${HEADERS_WIDGETS}
)

target_link_libraries(tablelibs QHexView Qt6::Widgets Qt6::Gui Qt6::WebChannel Qt6::WebEngineCore Qt6::WebEngineWidgets Qt6::Positioning Qt6::Core5Compat)
target_include_directories(tablelibs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${SUPPORT_INCLUDE_DIR})

if(WIN32)
target_compile_options(tablelibs PRIVATE /MP )

target_link_libraries(tablelibs
    debug ${CMAKE_CURRENT_SOURCE_DIR}/../support_package/debug/archive.lib
    debug ${CMAKE_CURRENT_SOURCE_DIR}/../support_package/debug/libmagic.lib
    optimized ${CMAKE_CURRENT_SOURCE_DIR}/../support_package/release/archive.lib
    optimized ${CMAKE_CURRENT_SOURCE_DIR}/../support_package/release/libmagic.lib
    icu.lib
)
endif(WIN32)
