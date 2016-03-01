message(STATUS "preparing stylelib")

set(PNAME dspstylelib)
if (QT5BUILD)
    set(PNAME dspstylelib_qt5)
endif (QT5BUILD)
project(${PNAME})

file(GLOB styleSRCS *.cpp)
file(GLOB styleHDRS *.h)
set(styleMOC_HDRS animhandler.h handlers.h progresshandler.h widgets.h stackanimator.h windowdata.h)

if (QTDBUS_FOUND)
    set(styleSRCS ${styleSRCS} macmenu.cpp)
    set(styleHDRS ${styleHDRS} macmenu.h macmenu-dbus.h)
    set(styleMOC_HDRS ${styleMOC_HDRS} macmenu.h macmenu-dbus.h)
endif (QTDBUS_FOUND)

if (QT5BUILD)
    qt5_wrap_cpp(styleMOC_SRCS ${styleMOC_HDRS})
else (QT5BUILD)
    qt4_wrap_cpp(styleMOC_SRCS ${styleMOC_HDRS})
endif (QT5BUILD)

add_library(${PNAME} SHARED ${styleSRCS} ${styleHDRS} ${styleMOC_SRCS})

if (QT5BUILD)
    target_link_libraries(${PNAME} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus Qt5::X11Extras dspsettings_qt5)
else (QT5BUILD)
    target_link_libraries(${PNAME} ${QT_LIBRARIES} ${QT_QTDBUS_LIBRARY} dspsettings)
endif (QT5BUILD)

if (XCB_FOUND)
    target_link_libraries(${PNAME} XCB::XCB)
endif (XCB_FOUND)

if (X11_FOUND)
    target_link_libraries(${PNAME} ${X11_LIBRARIES})
endif (X11_FOUND)

install(TARGETS ${PNAME} LIBRARY DESTINATION ${LIBDIR})
#install(FILES ${sharedHDRS} DESTINATION include/dsp)
