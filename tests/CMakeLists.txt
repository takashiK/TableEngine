find_package(Qt6 REQUIRED COMPONENTS Test)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE TEST_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

qt_add_executable(tst_tengine
    ${TEST_SOURCES}
    ${TEST_HEADERS}
)

target_include_directories(tst_tengine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../support_package/include
)

target_link_libraries(tst_tengine PRIVATE
    tablelibs
    Qt6::Widgets
    Qt6::Test
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/../support_package/debug/gmockd.lib>
    $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/../support_package/release/gmock.lib>
)

if(MSVC)
    target_compile_options(tst_tengine PRIVATE /MP)
endif()

add_test(NAME tst_tengine
    COMMAND tst_tengine
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# On Windows the Qt DLLs and support_package DLLs must be on PATH at test runtime.
if(WIN32)
    get_target_property(_qt_qmake_exec Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qt_qmake_exec}" DIRECTORY)
    set(_sp_debug_dir "${CMAKE_CURRENT_SOURCE_DIR}/../support_package/debug")
    set(_sp_release_dir "${CMAKE_CURRENT_SOURCE_DIR}/../support_package/release")
    set_tests_properties(tst_tengine PROPERTIES
        ENVIRONMENT "PATH=${_qt_bin_dir};${_sp_debug_dir};${_sp_release_dir};$ENV{PATH}"
    )
endif()
