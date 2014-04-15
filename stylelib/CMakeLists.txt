project (stylelib)

add_definitions (${QT_DEFINITIONS})

set (libSRCS
    xhandler.cpp
    ops.cpp
    shadowhandler.cpp)
    
set (libHDRS
    xhandler.h
    ops.h
    shadowhandler.h)

find_package (Qt4 REQUIRED)
include (${QT_USE_FILE})

find_package (X11)

add_library (stylelib SHARED ${libSRCS} ${libHDRS})

target_link_libraries (stylelib ${QT_LIBRARIES} ${X11_LIBRARIES} ${X11_Xrender_LIB})

#${LIB_INSTALL_DIR}
install (TARGETS stylelib LIBRARY DESTINATION /usr/lib)