message(STATUS "preparing config")
set(PROJECT_NAME styleproject_config)
set(LIB_NAME styleproject_config_lib)

set(configSRCS
    styleconfig.cpp
    settings.cpp)
    
set(configHDRS
    styleconfig.h
    settings.h)

set(configMocHDRS
    styleconfig.h
    settings.h)

set(MAIN
    main.cpp)

qt4_add_resources(configSRCS)
qt4_wrap_ui(UI_HDRS config.ui)
qt4_wrap_cpp(MOC_SRCS ${configMocHDRS})

# standalone application to launch from terminal w/o going to systemsettings
project(${PROJECT_NAME})
add_executable(${PROJECT_NAME} main.cpp ${configSRCS} ${MOC_SRCS} ${UI_HDRS})
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES})
install(TARGETS ${PROJECT_NAME} DESTINATION ${QT_BINARY_DIR})

# shared library
project(${LIB_NAME})
add_library(${LIB_NAME} SHARED ${configSRCS} ${MOC_SRCS} ${UI_HDRS})
target_link_libraries(${LIB_NAME} ${QT_LIBRARIES})
install(TARGETS ${LIB_NAME} LIBRARY DESTINATION ${LIBRARY_DIR})

# the systemsettings module in kde
project(kde-style-module)
kde4_add_plugin(kstyle_styleproject_config ${configSRCS} ${UI_HDRS})
target_link_libraries(kstyle_styleproject_config ${QT_LIBRARIES})
install(TARGETS kstyle_styleproject_config DESTINATION ${PLUGIN_INSTALL_DIR})
