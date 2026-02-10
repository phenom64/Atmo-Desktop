message(STATUS "preparing config")

project(nsesettings LANGUAGES CXX)

add_library(nsesettings SHARED settings.cpp settings.h)
target_link_libraries(nsesettings
    PUBLIC
        Qt6::Core
        Qt6::Gui
    PRIVATE
        Qt6::Widgets
)
target_include_directories(nsesettings PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
install(TARGETS nsesettings LIBRARY DESTINATION ${KDE_INSTALL_LIBDIR})
install(FILES settings.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/nse)

add_executable(atmo_config main.cpp styleconfig.cpp styleconfig.h config.ui)
target_link_libraries(atmo_config
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        nsesettings
)
install(TARGETS atmo_config RUNTIME DESTINATION ${KDE_INSTALL_BINDIR})
