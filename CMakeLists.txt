cmake_minimum_required(VERSION 3.20)

project(Coollab)
set(COOL_APP_VERSION "1.2.0 MacOS")
add_executable(${PROJECT_NAME})
add_library(Coollab-Properties INTERFACE) # This is needed by both Coollab and its tests
target_link_libraries(${PROJECT_NAME} PRIVATE Coollab-Properties)

# Choose C++ version
target_compile_features(Coollab-Properties INTERFACE cxx_std_20)

# Set the folder where the executable is created
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

# Set warning level
if(MSVC)
    target_compile_options(Coollab-Properties INTERFACE /W4)
else()
    target_compile_options(Coollab-Properties INTERFACE -Wall -Wextra -Wpedantic -pedantic-errors -Wconversion -Wsign-conversion -Wimplicit-fallthrough)
endif()

# Maybe enable warnings as errors
set(WARNINGS_AS_ERRORS_FOR_COOLLAB OFF CACHE BOOL "ON iff you want to treat warnings as errors") # Might be overriden in the CMake cache

if(WARNINGS_AS_ERRORS_FOR_COOLLAB)
    if(MSVC)
        target_compile_options(Coollab-Properties INTERFACE /WX)
    else()
        target_compile_options(Coollab-Properties INTERFACE -Werror)
    endif()
endif()

# Embed the manifest file into the executable. This manifest tells windows that our app supports long paths
if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/app-resources/Coollab.manifest")
endif()

# Include Cool
# Choose our GPU API and its version
# set(COOL_USE_VULKAN 110)
set(COOL_USE_OPENGL $<IF:$<PLATFORM_ID:Darwin>,410,430>) # Must be >= 330 for ImGui to work properly # We use 410 when building on Mac because it doesn't support higher versions of OpenGL
set(COOL_USE_MACOS_BUNDLE ON)
set(COOL_APP_NAME "Coollab")
add_subdirectory(Cool)
target_link_libraries(Coollab-Properties INTERFACE Cool::Core)
cool_setup(${PROJECT_NAME})
Cool__set_app_icon(${PROJECT_NAME} "res/logo.png" "${CMAKE_SOURCE_DIR}/app-resources/icon.rc")

# Include cmd
add_subdirectory(lib/cmd)
target_link_libraries(Coollab-Properties INTERFACE cmd::cmd)

# Include range-v3
target_include_directories(Coollab-Properties SYSTEM INTERFACE lib/range-v3/include)
install(FILES "lib/range-v3/LICENSE.txt" DESTINATION "license/range-v3")

set(COOLLAB_REQUIRE_ALL_FEATURES OFF CACHE BOOL "Some features are optional, like making HTTPS requests to post images to the Coollab gallery. If you don't have the necessary dependencies these features will be disabled. If you want to make sure you have all the features of Coollab enabled, set this option to ON.")

# -----------------
# ---cpp-httplib---
# -----------------
set(OPENSSL_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(HTTPLIB_INSTALL OFF CACHE BOOL "" FORCE)

if(COOLLAB_REQUIRE_ALL_FEATURES)
    set(HTTPLIB_REQUIRE_OPENSSL ON CACHE BOOL "" FORCE)
    set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
else()
    set(HTTPLIB_REQUIRE_OPENSSL OFF CACHE BOOL "" FORCE)
    set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE ON CACHE BOOL "" FORCE)
endif()

add_subdirectory(lib/cpp-httplib)
target_link_libraries(Coollab-Properties INTERFACE httplib)
install(FILES "lib/cpp-httplib/LICENSE" DESTINATION "license/cpp-httplib")
install(FILES "lib/openssl/LICENSE.txt" DESTINATION "license/openssl")

# Grab all the source files
file(GLOB_RECURSE COOLLAB_SOURCES CONFIGURE_DEPENDS src/*)
target_sources(${PROJECT_NAME} PRIVATE ${COOLLAB_SOURCES})

# Set include directories
target_include_directories(Coollab-Properties INTERFACE src)
target_include_directories(Coollab-Properties SYSTEM INTERFACE lib)

# Copy resources to the output folder (where the executable is created) after each build
# and register them for installation when creating the installer.
include("Cool/CMakeUtils/files_and_folders.cmake")

function(Lab_copy_folder FOLDER_RELATIVE_PATH)
    Cool__target_copy_folder(${PROJECT_NAME} ${FOLDER_RELATIVE_PATH})
    install(DIRECTORY ${FOLDER_RELATIVE_PATH} DESTINATION bin)
endfunction()

Lab_copy_folder("res")
Lab_copy_folder("User data Default")
Lab_copy_folder("User data Init")
Lab_copy_folder("Nodes")

# License
install(FILES "LICENSE.txt" DESTINATION "license" RENAME "Coollab-LICENSE.txt")
Cool__target_copy_file(${PROJECT_NAME} LICENSE.txt)

if(MSVC)
    target_compile_options(Coollab-Properties INTERFACE /bigobj) # We use too many templates apparently so we need this flag O:)
endif()

# ---------------------
# ---Setup the tests---
# ---------------------
list(REMOVE_ITEM COOLLAB_SOURCES ${CMAKE_SOURCE_DIR}/src/main.cpp)
add_executable(Tests-Coollab tests/tests.cpp ${COOLLAB_SOURCES})
target_compile_definitions(Tests-Coollab PRIVATE LAB_ENABLE_TESTS)
target_link_libraries(Tests-Coollab PRIVATE Coollab-Properties)
target_link_libraries(Tests-Coollab PRIVATE doctest::doctest)
set_target_properties(Tests-Coollab PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/tests/${CMAKE_BUILD_TYPE})
cool_setup(Tests-Coollab)
