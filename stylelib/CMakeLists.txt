message(STATUS "preparing stylelib")
project(stylelib)

set(libSRCS
    xhandler.cpp
    ops.cpp
    shadowhandler.cpp
    render.cpp
    color.cpp
    progresshandler.cpp
    animhandler.cpp
    handlers.cpp
    widgets.cpp)
    
set(libHDRS
    xhandler.h
    ops.h
    shadowhandler.h
    render.h
    macros.h
    color.h
    progresshandler.h
    animhandler.h
    handlers.h
    widgets.h)

set(libMocHDRS
    progresshandler.h
    animhandler.h
    handlers.h
    ops.h
    widgets.h
    xhandler.h)

set(QT_USE_QTDBUS TRUE)
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

qt4_wrap_cpp(MOC_SRCS ${libMocHDRS})

find_package(X11)

add_library(stylelib SHARED ${libSRCS} ${libHDRS} ${MOC_SRCS})

#${X11_Xrender_LIB}
target_link_libraries(stylelib styleproject_config_lib ${QT_LIBRARIES} ${X11_LIBRARIES})

#${LIB_INSTALL_DIR}
install(TARGETS stylelib LIBRARY DESTINATION ${LIBRARY_DIR})
