message(STATUS "preparing config")

if (QT5BUILD)
    set(PRO_NAME styleproject_config_qt5)
else (QT5BUILD)
    set(PRO_NAME styleproject_config)
endif (QT5BUILD)
    
file(GLOB configSRCS *.cpp)
file(GLOB configHDRS *.h)

if (QT5BUILD)
#     qt5_add_resources(${configSRCS})
    qt5_wrap_ui(UI_HDRS config.ui)
    qt5_wrap_cpp(MOC_SRCS ${configHDRS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
else (QT5BUILD)
#     qt4_add_resources(${configSRCS})
    qt4_wrap_ui(UI_HDRS config.ui)
    qt4_wrap_cpp(MOC_SRCS ${configHDRS})
endif (QT5BUILD)

# standalone application to launch from terminal w/o going to systemsettings
project(${PRO_NAME})
add_executable(${PRO_NAME} ${UI_HDRS} ${configSRCS} ${MOC_SRCS} ${configHDRS})

if (QT5BUILD)
    target_link_libraries(${PRO_NAME} Qt5::Core Qt5::Gui Qt5::Widgets)
else (QT5BUILD)
    target_link_libraries(${PRO_NAME} ${QT_LIBRARIES})
endif (QT5BUILD)
install(TARGETS ${PRO_NAME} DESTINATION bin)

# the systemsettings module in kde
if (NOT QT5BUILD AND KDE4_FOUND)
    project(kstyle_styleproject_config)
    kde4_add_plugin(kstyle_styleproject_config ${configSRCS} ${UI_HDRS})
    target_link_libraries(kstyle_styleproject_config ${QT_LIBRARIES})
    install(TARGETS kstyle_styleproject_config DESTINATION ${PLUGIN_INSTALL_DIR})
endif (NOT QT5BUILD AND KDE4_FOUND)
