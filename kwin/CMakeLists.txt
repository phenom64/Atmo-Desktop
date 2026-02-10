message(STATUS "preparing kwin deco")

find_package(KDecoration3 QUIET)
if(NOT KDecoration3_FOUND)
    message(WARNING "KDecoration3 not found. Skipping Atmo decoration plugin (Plasma 6).")
    return()
endif()

find_package(KF6CoreAddons REQUIRED)
find_package(KF6Config REQUIRED)
find_package(KF6ConfigWidgets REQUIRED)
find_package(KF6WindowSystem REQUIRED)
find_package(KF6GuiAddons REQUIRED)
find_package(KF6KCMUtils REQUIRED)
find_package(KF6I18n REQUIRED)

set(ATMO_KDECORATION_TARGET "")
if(TARGET KDecoration3::KDecoration)
    set(ATMO_KDECORATION_TARGET KDecoration3::KDecoration)
elseif(TARGET KDecoration3::KDecoration3)
    set(ATMO_KDECORATION_TARGET KDecoration3::KDecoration3)
endif()

if(ATMO_KDECORATION_TARGET STREQUAL "")
    message(FATAL_ERROR "KDecoration3 was found, but no usable CMake target was exported.")
endif()

find_package(dbusmenu-qt6 QUIET)
if(NOT dbusmenu-qt6_FOUND)
    message(STATUS "dbusmenu-qt6 not found: titlebar DBusMenu integration will be disabled.")
endif()

set(SRCS
    kwinclient2.cpp
    decobutton.cpp
)
set(HDRS
    kwinclient2.h
    decobutton.h
    decoadaptor.h
    kdecoration_compat.h
)

if(ATMO_HAVE_XCB)
    list(APPEND SRCS xcbatoms.cpp)
    list(APPEND HDRS xcbatoms.h)
endif()

if(dbusmenu-qt6_FOUND)
    list(APPEND SRCS menubar.cpp)
    list(APPEND HDRS menubar.h)
endif()

add_library(atmodecoration MODULE ${SRCS} ${HDRS})
target_compile_definitions(atmodecoration PRIVATE ATMO_USE_KDECORATION3=1)

target_link_libraries(atmodecoration
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        nsesettings
        atmolib
    PRIVATE
        ${ATMO_KDECORATION_TARGET}
        KF6::ConfigCore
        KF6::CoreAddons
        KF6::ConfigWidgets
        KF6::GuiAddons
        KF6::WindowSystem
        KF6::KCMUtils
        KF6::I18n
        ${ATMO_X11_LIBS}
)

if(TARGET Qt6::DBus)
    target_link_libraries(atmodecoration PUBLIC Qt6::DBus)
endif()

if(dbusmenu-qt6_FOUND)
    target_link_libraries(atmodecoration PUBLIC dbusmenu-qt6)
    target_compile_definitions(atmodecoration PRIVATE HASDBUSMENU=1)
endif()

install(TARGETS atmodecoration DESTINATION ${KDE_INSTALL_PLUGINDIR}/org.kde.kdecoration3)
