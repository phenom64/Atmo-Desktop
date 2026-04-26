cmake_minimum_required(VERSION 3.20)

project(atmo_manager LANGUAGES CXX)

add_executable(atmo_manager main.cpp)
target_link_libraries(atmo_manager
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        nsesettings
        atmolib
)
set_target_properties(atmo_manager PROPERTIES
    BUILD_RPATH "${CMAKE_BINARY_DIR};${CMAKE_BINARY_DIR}/atmolib"
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_LIBDIR}"
)
install(TARGETS atmo_manager RUNTIME DESTINATION ${KDE_INSTALL_BINDIR})
