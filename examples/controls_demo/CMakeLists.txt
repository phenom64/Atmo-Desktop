cmake_minimum_required(VERSION 3.20)

project(atmo_demo_controls LANGUAGES CXX)

add_executable(atmo_demo_controls main.cpp)
target_link_libraries(atmo_demo_controls PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
install(TARGETS atmo_demo_controls RUNTIME DESTINATION ${KDE_INSTALL_BINDIR})
