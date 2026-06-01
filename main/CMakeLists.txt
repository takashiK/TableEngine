
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

# --- Installation rules (used by cmake --install and CPack) ---
if(WIN32)
    install(TARGETS TableEngine RUNTIME DESTINATION .)

    install(FILES
        "${CMAKE_SOURCE_DIR}/LICENSE"
        "${CMAKE_SOURCE_DIR}/README.md"
        DESTINATION .
    )

    # Run windeployqt6 after installing the exe to collect all Qt runtime DLLs
    find_program(WINDEPLOYQT6_EXECUTABLE
        NAMES windeployqt6 windeployqt
        PATHS "${Qt6_DIR}/../../../bin"
        NO_DEFAULT_PATH
    )
    if(WINDEPLOYQT6_EXECUTABLE)
        install(CODE "
            message(STATUS \"Running windeployqt6 to collect Qt runtime files...\")
            execute_process(
                COMMAND \"${WINDEPLOYQT6_EXECUTABLE}\"
                    --release
                    --no-translations
                    --no-quick-import
                    --no-opengl-sw
                    --no-ffmpeg
                    --skip-plugin-types qmltooling
                    --exclude-plugins qtposition_nmea
                    --dir \"\${CMAKE_INSTALL_PREFIX}\"
                    \"\${CMAKE_INSTALL_PREFIX}/TableEngine.exe\"
                RESULT_VARIABLE _wdq_result
            )
            if(NOT _wdq_result EQUAL 0)
                message(WARNING \"windeployqt6 returned: \${_wdq_result}\")
            endif()
        ")
    else()
        message(WARNING "windeployqt6 not found - Qt runtime DLLs will not be collected automatically")
    endif()
endif(WIN32)