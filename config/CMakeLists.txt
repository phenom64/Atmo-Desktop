message(STATUS "preparing config")

if (QT5BUILD)
    set(PRO_NAME styleproject_config_qt5)
    set(LIB_NAME styleproject_config_lib_qt5)
else (QT5BUILD)
    set(PRO_NAME styleproject_config)
    set(LIB_NAME styleproject_config_lib)
endif (QT5BUILD)
    
file(GLOB configSRCS *.cpp)
file(GLOB configHDRS *.h)
set(configMocHDRS styleconfig.h settings.h)

if (QT5BUILD)
    qt5_add_resources(configSRCS)
    qt5_wrap_ui(UI_HDRS config.ui)
    qt5_wrap_cpp(MOC_SRCS ${configMocHDRS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
else (QT5BUILD)
    qt4_add_resources(configSRCS)
    qt4_wrap_ui(UI_HDRS config.ui)
    qt4_wrap_cpp(MOC_SRCS ${configMocHDRS})
endif (QT5BUILD)

# standalone application to launch from terminal w/o going to systemsettings
project(${PRO_NAME})
add_executable(${PRO_NAME} ${configSRCS} ${MOC_SRCS})
if (QT5BUILD)
    target_link_libraries(${PRO_NAME} Qt5::Core Qt5::Gui Qt5::Widgets)
else (QT5BUILD)
    target_link_libraries(${PRO_NAME} ${QT_LIBRARIES})
endif (QT5BUILD)
install(TARGETS ${PRO_NAME} DESTINATION bin)

# shared library
project(${LIB_NAME})
add_library(${LIB_NAME} SHARED ${configSRCS} ${MOC_SRCS})

if (QT5BUILD)
    target_link_libraries(${LIB_NAME} Qt5::Core Qt5::Gui Qt5::Widgets)
else (QT5BUILD)
    target_link_libraries(${LIB_NAME} ${QT_LIBRARIES})
endif (QT5BUILD)
install(TARGETS ${LIB_NAME} LIBRARY DESTINATION lib)

# the systemsettings module in kde
if (NOT QT5BUILD)
    project(kstyle_styleproject_config)
    kde4_add_plugin(kstyle_styleproject_config ${configSRCS} ${UI_HDRS})
    target_link_libraries(kstyle_styleproject_config ${QT_LIBRARIES})
    install(TARGETS kstyle_styleproject_config DESTINATION ${PLUGIN_INSTALL_DIR})
endif (NOT QT5BUILD)
