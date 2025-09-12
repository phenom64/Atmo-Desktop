cmake_minimum_required(VERSION 3.5)

project(kcm_atmo)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

set(SRCS kcm_atmo.cpp)

add_library(kcm_atmo MODULE ${SRCS})
target_link_libraries(kcm_atmo Qt5::Core Qt5::Gui Qt5::Widgets KF5::KCMUtils KF5::ConfigWidgets KF5::I18n nsesettings atmolib)

set_target_properties(kcm_atmo PROPERTIES OUTPUT_NAME kcm_atmo)

install(TARGETS kcm_atmo DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES kcm_atmo.json DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

