message(STATUS "preparing atmolib")

project(atmolib)

file(GLOB styleSRCS *.cpp)
file(GLOB styleHDRS *.h)
set(styleMOC_HDRS animhandler.h handlers.h progresshandler.h widgets.h stackanimator.h windowdata.h titlewidget.h toolbarhelpers.h windowhelpers.h)

if (QTDBUS_FOUND)
    set(styleSRCS ${styleSRCS} macmenu.cpp)
    set(styleHDRS ${styleHDRS} macmenu.h macmenu-dbus.h)
    set(styleMOC_HDRS ${styleMOC_HDRS} macmenu.h macmenu-dbus.h)
endif ()

qt5_wrap_cpp(styleMOC_SRCS ${styleMOC_HDRS})

add_library(atmolib SHARED ${styleSRCS} ${styleHDRS} ${styleMOC_SRCS})

target_link_libraries(atmolib Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus Qt5::X11Extras nsesettings)

if (XCB_FOUND)
    target_link_libraries(atmolib XCB::XCB)
endif ()

if (X11_FOUND)
    target_link_libraries(atmolib ${X11_LIBRARIES})
endif ()

install(TARGETS atmolib LIBRARY DESTINATION ${LIBDIR})
