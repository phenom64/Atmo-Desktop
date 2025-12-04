cmake_minimum_required(VERSION 3.5)

# Pretty build banner (inspired by OpenXMB)
string(ASCII 27 ESC)
set(COLOR_RESET   "${ESC}[0m")
set(COLOR_CYAN    "${ESC}[34m")
set(COLOR_RED     "${ESC}[31m")
set(COLOR_YELLOW  "${ESC}[33m")
set(COLOR_GREEN   "${ESC}[32m")

project(atmo VERSION 1.0.0)

message(STATUS "${COLOR_GREEN}Starting NSE.AtmoFramework Builder...${COLOR_RESET}")
message(STATUS "${COLOR_YELLOW}TM & (C) 2025 Syndromatic Ltd. All rights reserved.${COLOR_RESET}")
message(STATUS "${COLOR_YELLOW}Designed by Kavish Krishnakumar in Manchester.${COLOR_RESET}")

# bake runtime search path so installed tools (e.g. atmo_manager) find libatmo/libatmolib
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Platform detection (lightweight)
if(APPLE)
  set(PLATFORM_NAME "macOS")
elseif(WIN32)
  set(PLATFORM_NAME "Windows")
elseif(UNIX)
  if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    if(EXISTS "/etc/os-release")
      file(READ "/etc/os-release" OS_RELEASE_CONTENT)
      string(REGEX MATCH "ID=ubuntu" IS_UBUNTU "${OS_RELEASE_CONTENT}")
      if(IS_UBUNTU)
        set(PLATFORM_NAME "SynOS/Linux")
      else()
        set(PLATFORM_NAME "Linux")
      endif()
    else()
      set(PLATFORM_NAME "Linux")
    endif()
  else()
    set(PLATFORM_NAME "Unix")
  endif()
else()
  set(PLATFORM_NAME "Unknown")
endif()
message(STATUS "Running on ${COLOR_CYAN}${PLATFORM_NAME}${COLOR_RESET}")

# Default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()
message(STATUS "${COLOR_CYAN}Building version ${PROJECT_VERSION} (${CMAKE_BUILD_TYPE})${COLOR_RESET}")
add_compile_definitions(ATMO_VERSION_STR="${PROJECT_VERSION}")

# IPO/LTO support (optional) like OpenXMB
if(POLICY CMP0069)
  cmake_policy(SET CMP0069 NEW)
endif()
include(CheckIPOSupported OPTIONAL RESULT_VARIABLE _IPO_MODULE)
if(_IPO_MODULE)
  check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
  if(IPO_SUPPORTED)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
  else()
    message(STATUS "${COLOR_YELLOW}IPO/LTO not supported: ${IPO_ERROR}${COLOR_RESET}")
  endif()
else()
  set(IPO_SUPPORTED OFF)
  set(IPO_ERROR "CheckIPOSupported.cmake not available for this CMake version")
  message(STATUS "${COLOR_YELLOW}IPO/LTO check unavailable: ${IPO_ERROR}${COLOR_RESET}")
endif()

option(DEBUG "verbose debugging messages in output" OFF)

if (DEBUG)
    add_definitions(-DDEBUG=1)
endif ()

if (NOT LIBDIR)
    set(LIBDIR lib)
endif ()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5X11Extras)
find_package(Qt5OpenGL)
find_package(Qt5DBus)
include_directories(${Qt5Widgets_INCLUDE_DIRS} ${Qt5X11Extras_INCLUDE_DIRS})
if (Qt5DBus_FOUND)
    include_directories(${Qt5DBus_INCLUDE_DIRS})
    add_definitions(${Qt5DBus_DEFINITIONS})
    set(QTDBUS_FOUND ON)
endif ()
add_definitions(${Qt5Widgets_DEFINITIONS})
find_package(ECM)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

if(Qt5Core_FOUND)
    message(STATUS "Using Qt ${COLOR_CYAN}${Qt5Core_VERSION}${COLOR_RESET}")
endif()

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

add_subdirectory(config)

# source
set(SRCS buttons.cpp contentssize.cpp events.cpp init.cpp inputs.cpp items.cpp overlay.cpp panels.cpp pixelmetric.cpp polish.cpp sliders.cpp stylehint.cpp nse.cpp subrects.cpp tabs.cpp)
set(HDRS overlay.h nse.h namespace.h)
set(MOC_HDRS overlay.h nse.h)

if (QTDBUS_FOUND)
    message(STATUS "Found QtDBus, building macmenu support")
    add_definitions(-DHASDBUS=1)
endif ()

qt5_wrap_cpp(MOC_SRCS ${MOC_HDRS})

add_library(${PROJECT_NAME} SHARED ${SRCS} ${MOC_SRCS} ${HDRS})

# xcb and x11
find_package(XCB COMPONENTS XCB)
if (XCB_FOUND)
    add_definitions(-DHASXCB=1)
    target_link_libraries(${PROJECT_NAME} XCB::XCB)
    include_directories(${XCB_INCLUDES})
else ()
    find_package(X11)
    if (X11_FOUND)
        add_definitions(-DHASX11=1)
        include_directories(${X11_INCLUDES})
        target_link_libraries(${PROJECT_NAME} ${X11_X11_LIB} ${X11_LIBRARIES})
    endif ()
endif ()

set(KF5_MIN_VERSION "5.9.0")
find_package(KF5 ${KF5_MIN_VERSION} COMPONENTS WindowSystem)
if (KF5_FOUND)
    include(KDEInstallDirs)
    include(KDECMakeSettings)
    include(KDECompilerSettings)
    target_link_libraries(${PROJECT_NAME} KF5::WindowSystem)
    add_definitions(-DHASKF5)
endif ()

add_subdirectory(atmolib)

# link
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus Qt5::X11Extras nsesettings atmolib)

# install style plugin to Qt plugins/styles
get_target_property(QMAKE Qt5::qmake IMPORTED_LOCATION)
execute_process(COMMAND ${QMAKE} -query QT_INSTALL_PLUGINS OUTPUT_VARIABLE PLUGINPATH OUTPUT_STRIP_TRAILING_WHITESPACE)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGINPATH}/styles)

# KWin decoration (KF5 only)
find_package(KDecoration2)
if (KDecoration2_FOUND)
    add_subdirectory(kwin)
endif ()

# Install default config to share for first-run provisioning
install(FILES ${CMAKE_SOURCE_DIR}/NSE.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/share/atmo)

# Optional components
option(BUILD_EXAMPLES "Build example applications" OFF)
option(BUILD_KCM "Build Plasma System Settings KCM (stub)" OFF)
option(BUILD_MANAGER "Build Atmo Framework Manager app" ON)

# Optional compile-time tuning
option(ATMO_ENABLE_NATIVE_OPTIMIZATIONS "Enable -march=native (or /arch:...)" OFF)
option(ATMO_ENABLE_FAST_MATH "Enable fast-math where safe" OFF)

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

# Greeter for options
if(BUILD_EXAMPLES)
  set(EX_COLOR "${COLOR_GREEN}")
else()
  set(EX_COLOR "${COLOR_RED}")
endif()
if(BUILD_KCM)
  set(KCM_COLOR "${COLOR_GREEN}")
else()
  set(KCM_COLOR "${COLOR_RED}")
endif()

if(IPO_SUPPORTED)
  set(LTO_COLOR "${COLOR_GREEN}")
else()
  set(LTO_COLOR "${COLOR_RED}")
endif()

if(ATMO_ENABLE_NATIVE_OPTIMIZATIONS)
  set(NAT_COLOR "${COLOR_GREEN}")
else()
  set(NAT_COLOR "${COLOR_RED}")
endif()

if(ATMO_ENABLE_FAST_MATH)
  set(FM_COLOR "${COLOR_GREEN}")
else()
  set(FM_COLOR "${COLOR_RED}")
endif()

message(STATUS "Features: EXAMPLES=${EX_COLOR}${BUILD_EXAMPLES}${COLOR_RESET} KCM=${KCM_COLOR}${BUILD_KCM}${COLOR_RESET} LTO=${LTO_COLOR}${IPO_SUPPORTED}${COLOR_RESET} NATIVE=${NAT_COLOR}${ATMO_ENABLE_NATIVE_OPTIMIZATIONS}${COLOR_RESET} FAST_MATH=${FM_COLOR}${ATMO_ENABLE_FAST_MATH}${COLOR_RESET}")

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (BUILD_KCM)
    find_package(KF5 COMPONENTS KCMUtils ConfigWidgets I18n CoreAddons)
    if (KF5_FOUND AND KF5KCMUtils_FOUND)
        add_subdirectory(kcm)
    else ()
        message(STATUS "KCM not built: KF5KCMUtils not found")
    endif ()
endif ()

if (BUILD_MANAGER)
    add_subdirectory(manager)
endif ()
