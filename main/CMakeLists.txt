
set(SOURCES main.cpp)

qt_add_resources(SOURCES TableEngine.qrc)

qt_add_executable(TableEngine
    ${SOURCES}
    TableEngine_resource.rc
)

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