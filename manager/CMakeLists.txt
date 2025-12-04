cmake_minimum_required(VERSION 3.5)

project(atmo_manager)

add_executable(atmo_manager main.cpp)
target_link_libraries(atmo_manager Qt5::Core Qt5::Gui Qt5::Widgets nsesettings atmolib)
set_target_properties(atmo_manager PROPERTIES
    BUILD_RPATH "${CMAKE_BINARY_DIR};${CMAKE_BINARY_DIR}/atmolib"
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
)
install(TARGETS atmo_manager RUNTIME DESTINATION bin)
