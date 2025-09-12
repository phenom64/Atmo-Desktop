cmake_minimum_required(VERSION 3.5)

project(atmo_demo_controls)

add_executable(atmo_demo_controls main.cpp)
target_link_libraries(atmo_demo_controls Qt5::Core Qt5::Gui Qt5::Widgets)
install(TARGETS atmo_demo_controls RUNTIME DESTINATION bin)

