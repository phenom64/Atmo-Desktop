message(STATUS "preparing stylelib")

if (QT5BUILD)
    set(STYLELIB stylelib_qt5)
else (QT5BUILD)
    set(STYLELIB stylelib)
endif (QT5BUILD)

project(${STYLELIB})

file(GLOB libSRCS *.cpp)
file(GLOB libHDRS *.h)
set(libMocHDRS progresshandler.h animhandler.h handlers.h ops.h widgets.h)

if (QT5BUILD)
    qt5_wrap_cpp(MOC_SRCS ${libMocHDRS})
else (QT5BUILD)
    qt4_wrap_cpp(MOC_SRCS ${libMocHDRS})
endif (QT5BUILD)

add_library(${STYLELIB} SHARED ${libSRCS} ${libHDRS} ${MOC_SRCS})

if (QT5BUILD)
    target_link_libraries(${STYLELIB} styleproject_config_lib_qt5 Qt5::Core Qt5::Gui Qt5::Widgets ${X11_X11_LIB} ${X11_LIBRARIES})
else (QT5BUILD)
    target_link_libraries(${STYLELIB} styleproject_config_lib ${QT_LIBRARIES} ${X11_LIBRARIES})
endif (QT5BUILD)

install(TARGETS ${STYLELIB} LIBRARY DESTINATION ${LIBRARY_DIR})
