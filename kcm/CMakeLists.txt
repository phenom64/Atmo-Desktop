cmake_minimum_required(VERSION 3.20)

project(kcm_atmo LANGUAGES CXX)

add_library(kcm_atmo MODULE kcm_atmo.cpp)
target_link_libraries(kcm_atmo
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        KF6::KCMUtils
        KF6::ConfigWidgets
        KF6::I18n
        KF6::CoreAddons
        nsesettings
        atmolib
)

set_target_properties(kcm_atmo PROPERTIES OUTPUT_NAME kcm_atmo)

# Plasma 6 System Settings module plugin path
install(TARGETS kcm_atmo DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings)
# KDecoration3 loads its config module from a dedicated plugin namespace.
install(TARGETS kcm_atmo DESTINATION ${KDE_INSTALL_PLUGINDIR}/org.kde.kdecoration3.kcm)
set(ATMO_KCM_METADATA_DIR "${KDE_INSTALL_KSERVICESDIR}")
if(ATMO_KCM_METADATA_DIR STREQUAL "")
    set(ATMO_KCM_METADATA_DIR "${KDE_INSTALL_DATADIR}/kservices6")
endif()
install(FILES kcm_atmo.json DESTINATION ${ATMO_KCM_METADATA_DIR})
