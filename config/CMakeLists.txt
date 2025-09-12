message(STATUS "preparing config")

# shared settings lib
project(nsesettings)
set(sharedSRCS settings.cpp)
set(sharedHDRS settings.h)
add_library(nsesettings SHARED ${sharedSRCS} ${sharedHDRS})
target_link_libraries(nsesettings Qt5::Core)
install(TARGETS nsesettings LIBRARY DESTINATION ${LIBDIR})
install(FILES ${sharedHDRS} DESTINATION include/nse)

set(PRO_NAME atmo_config)
set(configSRCS main.cpp styleconfig.cpp)
set(configHDRS styleconfig.h)
qt5_wrap_ui(UI_HDRS config.ui)
qt5_wrap_cpp(MOC_SRCS ${configHDRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# standalone application to launch from terminal
project(${PRO_NAME})
add_executable(${PRO_NAME} ${UI_HDRS} ${configSRCS} ${MOC_SRCS} ${configHDRS})
target_link_libraries(${PRO_NAME} Qt5::Core Qt5::Gui Qt5::Widgets nsesettings)
install(TARGETS ${PRO_NAME} DESTINATION bin)
