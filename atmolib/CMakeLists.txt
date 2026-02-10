message(STATUS "preparing atmolib")

project(atmolib LANGUAGES CXX)

file(GLOB styleSRCS CONFIGURE_DEPENDS *.cpp)
file(GLOB styleHDRS CONFIGURE_DEPENDS *.h)

# macmenu integration is DBus-driven; keep optional
if(NOT QTDBUS_FOUND)
    list(REMOVE_ITEM styleSRCS "${CMAKE_CURRENT_SOURCE_DIR}/macmenu.cpp")
    list(REMOVE_ITEM styleHDRS "${CMAKE_CURRENT_SOURCE_DIR}/macmenu.h" "${CMAKE_CURRENT_SOURCE_DIR}/macmenu-dbus.h")
endif()

add_library(atmolib SHARED ${styleSRCS} ${styleHDRS})
target_include_directories(atmolib
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
)

target_link_libraries(atmolib
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        nsesettings
        ${ATMO_X11_LIBS}
)

if(TARGET Qt6::DBus)
    target_link_libraries(atmolib PRIVATE Qt6::DBus)
endif()

install(TARGETS atmolib LIBRARY DESTINATION ${KDE_INSTALL_LIBDIR})
