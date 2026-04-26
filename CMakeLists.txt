cmake_minimum_required(VERSION 3.20)

project(atmo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(DEBUG "Verbose debugging messages in output" OFF)
option(ATMO_BUILD_DECORATION "Build KWin decoration plugin" ON)
option(ATMO_ENABLE_X11_FALLBACK "Enable optional X11/XCB fallback path" ON)
option(ATMO_BUILD_KCM "Build Plasma System Settings KCM" ON)
option(BUILD_EXAMPLES "Build example applications" OFF)
option(BUILD_MANAGER "Build Atmo Framework Manager app" ON)
option(ATMO_ENABLE_NATIVE_OPTIMIZATIONS "Enable -march=native (or /arch:...)" OFF)
option(ATMO_ENABLE_FAST_MATH "Enable fast-math where safe" OFF)

if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
if(IPO_SUPPORTED)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
endif()

find_package(ECM 6.24 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(ATMO_ENABLE_NATIVE_OPTIMIZATIONS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        add_compile_options(-march=native)
    elseif(MSVC)
        add_compile_options(/arch:AVX2)
    endif()
endif()

if(ATMO_ENABLE_FAST_MATH)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        add_compile_options(-ffast-math)
    elseif(MSVC)
        add_compile_options(/fp:fast)
    endif()
endif()

if(DEBUG)
    add_compile_definitions(DEBUG=1)
endif()

add_compile_definitions(ATMO_VERSION_STR="${PROJECT_VERSION}")

# Core dependencies (Qt 6 / KF6 baseline)
find_package(Qt6 6.10 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Qt6 6.10 QUIET COMPONENTS DBus OpenGL)
find_package(KF6WindowSystem 6.24 REQUIRED)

set(QTDBUS_FOUND FALSE)
if(TARGET Qt6::DBus)
    set(QTDBUS_FOUND TRUE)
    add_compile_definitions(HASDBUS=1)
endif()

add_compile_definitions(HASKF6=1)

# Optional X11/XCB fallback. Wayland remains first-class path.
set(ATMO_HAVE_XCB FALSE)
set(ATMO_HAVE_X11 FALSE)
set(ATMO_X11_LIBS "")

if(ATMO_ENABLE_X11_FALLBACK)
    find_package(XCB QUIET COMPONENTS XCB)
    if(XCB_FOUND)
        set(ATMO_HAVE_XCB TRUE)
        list(APPEND ATMO_X11_LIBS XCB::XCB)
        add_compile_definitions(HASXCB=1)
    else()
        find_package(X11 QUIET)
        if(X11_FOUND)
            set(ATMO_HAVE_X11 TRUE)
            list(APPEND ATMO_X11_LIBS X11::X11)
            add_compile_definitions(HASX11=1)
        endif()
    endif()
endif()

add_compile_definitions(ATMO_ENABLE_X11_FALLBACK=$<BOOL:${ATMO_ENABLE_X11_FALLBACK}>)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

if(NOT TARGET uninstall)
    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

# Runtime search path for helper apps that link libatmo/atmolib
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_LIBDIR}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_RPATH "${CMAKE_BINARY_DIR};${CMAKE_BINARY_DIR}/atmolib")

add_subdirectory(config)
add_subdirectory(atmolib)

set(ATMO_STYLE_SRCS
    buttons.cpp
    contentssize.cpp
    events.cpp
    init.cpp
    inputs.cpp
    items.cpp
    overlay.cpp
    panels.cpp
    pixelmetric.cpp
    polish.cpp
    sliders.cpp
    stylehint.cpp
    nse.cpp
    subrects.cpp
    tabs.cpp
)

set(ATMO_STYLE_HDRS
    overlay.h
    nse.h
    namespace.h
)

add_library(${PROJECT_NAME} MODULE ${ATMO_STYLE_SRCS} ${ATMO_STYLE_HDRS})
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        KF6::WindowSystem
        nsesettings
        atmolib
        ${ATMO_X11_LIBS}
)

if(TARGET Qt6::DBus)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::DBus)
endif()

# Install style plugin to Qt6 plugin styles dir.
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/styles
)

# Install default config template
install(FILES ${CMAKE_SOURCE_DIR}/NSE.conf DESTINATION ${KDE_INSTALL_DATADIR}/atmo)

if(ATMO_BUILD_DECORATION)
    add_subdirectory(kwin)
endif()

if(ATMO_BUILD_KCM)
    find_package(KF6KCMUtils 6.24 REQUIRED)
    find_package(KF6ConfigWidgets 6.24 REQUIRED)
    find_package(KF6I18n 6.24 REQUIRED)
    find_package(KF6CoreAddons 6.24 REQUIRED)
    add_subdirectory(kcm)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_MANAGER)
    add_subdirectory(manager)
endif()

message(STATUS "Atmo version: ${PROJECT_VERSION}")
message(STATUS "Qt version: ${Qt6Core_VERSION}")
message(STATUS "X11 fallback enabled: ${ATMO_ENABLE_X11_FALLBACK}")
message(STATUS "XCB backend available: ${ATMO_HAVE_XCB}")
message(STATUS "X11 backend available: ${ATMO_HAVE_X11}")
message(STATUS "Decoration build requested: ${ATMO_BUILD_DECORATION}")
message(STATUS "KCM build requested: ${ATMO_BUILD_KCM}")
