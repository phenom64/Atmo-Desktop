cmake_minimum_required(VERSION 3.5)

project(atmo_manager)

add_executable(atmo_manager main.cpp)
target_link_libraries(atmo_manager Qt5::Core Qt5::Gui Qt5::Widgets nsesettings)
install(TARGETS atmo_manager RUNTIME DESTINATION bin)

