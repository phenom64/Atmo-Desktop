project (stylelib)

add_definitions (${QT_DEFINITIONS})

set (libSRCS
    xhandler.cpp
    ops.cpp
    shadowhandler.cpp
    render.cpp
    color.cpp
    progresshandler.cpp)
    
set (libHDRS
    xhandler.h
    ops.h
    shadowhandler.h
    render.h
    macros.h
    color.h
    progresshandler.h)

set (libMocHDRS
    progresshandler.h
    ops.h)

set (LIBRARY_DIR /usr/lib)
set (QT_USE_QTDBUS TRUE)
find_package (Qt4 REQUIRED)
include (${QT_USE_FILE})

qt4_wrap_cpp (MOC_SRCS ${libMocHDRS})

find_package (X11)

add_library (stylelib SHARED ${libSRCS} ${libHDRS} ${MOC_SRCS})

target_link_libraries (stylelib ${QT_LIBRARIES} ${X11_LIBRARIES} ${X11_Xrender_LIB})

#${LIB_INSTALL_DIR}
install (TARGETS stylelib LIBRARY DESTINATION ${LIBRARY_DIR})
