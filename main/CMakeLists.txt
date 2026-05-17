
# Generate version.h from the VERSION field in the root CMakeLists.txt
configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h @ONLY)

set(SOURCES main.cpp)

qt_add_resources(SOURCES TableEngine.qrc)

# On Windows, generate the resource file from the template so the version
# number is injected automatically at configure time.
if(WIN32)
    configure_file(TableEngine_resource.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/TableEngine_resource.rc @ONLY)
    list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/TableEngine_resource.rc)
endif()

qt_add_executable(TableEngine
    ${SOURCES}
)

# Make the generated version.h visible to main.cpp
target_include_directories(TableEngine PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(TableEngine PRIVATE tablelibs)

set_target_properties(TableEngine PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

if(WIN32)
add_custom_command(TARGET TableEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/../support_package/$<IF:$<CONFIG:Debug>,debug/archive.dll,release/archive.dll>"
    "$<TARGET_FILE_DIR:TableEngine>/"
)

add_custom_command(TARGET TableEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/../support_package/$<IF:$<CONFIG:Debug>,debug/libmagic.dll,release/libmagic.dll>"
    "$<TARGET_FILE_DIR:TableEngine>/"
)
endif(WIN32)