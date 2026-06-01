find_package(Qt6 REQUIRED COMPONENTS Test)
find_package(GTest CONFIG REQUIRED)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE TEST_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

qt_add_executable(tst_tengine
    ${TEST_SOURCES}
    ${TEST_HEADERS}
)

target_include_directories(tst_tengine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(tst_tengine PRIVATE
    tablelibs
    Qt6::Widgets
    Qt6::Test
    GTest::gmock
)

if(MSVC)
    target_compile_options(tst_tengine PRIVATE /MP)
endif()

add_test(NAME tst_tengine
    COMMAND tst_tengine
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
