cmake_minimum_required(VERSION 3.16)
project(pypilot_settings LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(pypilot_settings INTERFACE)
target_include_directories(pypilot_settings INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

include(CTest)
enable_testing()

function(add_settings_test target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE pypilot_settings)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

add_settings_test(test_settings_catalog tests/test_settings_catalog.cpp)
add_settings_test(test_memory_store tests/test_memory_store.cpp)
add_settings_test(test_file_store tests/test_file_store.cpp)
add_settings_test(test_scope_helpers tests/test_scope_helpers.cpp)
add_settings_test(test_validation tests/test_validation.cpp)

add_executable(settings_example examples/SettingsExample/SettingsExample.cpp)
target_link_libraries(settings_example PRIVATE pypilot_settings)
