message(STATUS "preparing kwin deco")

    #NOTE: most of this copied from the default breeze deco and adapted for Atmo
    find_package(KF5 REQUIRED COMPONENTS CoreAddons ConfigWidgets WindowSystem GuiAddons)

    include(KDEInstallDirs)
    include(KDECMakeSettings)
    include(KDECompilerSettings)
    include(GenerateExportHeader)

    execute_process(COMMAND kf5-config --install module OUTPUT_VARIABLE MODULEPATH OUTPUT_STRIP_TRAILING_WHITESPACE)

    find_package(dbusmenu-qt5)

#     find_path(EFFECTS_H kwineffects.h PATH_SUFFIXES kf5)
#     
#     if (EFFECTS_H)
#         include_directories(${EFFECTS_H})
#     else (EFFECTS_H)
#         message(STATUS "didnt find kwineffects.h, not building effects")
#     endif (EFFECTS_H)
#     
#     find_library(KWIN_EFFECTS NAMES kwineffects PATH_SUFFIXES kf5)
#     find_library(KWIN_GLUTILS NAMES kwinglutils PATH_SUFFIXES kf5)
#     find_library(OPENGL NAMES GL)
# 
#     if (NOT KWIN_EFFECTS)
#         message(STATUS "didnt find kwineffects lib, not building effects")
#     endif (NOT KWIN_EFFECTS)
# 
#     if (NOT KWIN_GLUTILS)
#         message(STATUS "didnt find kwin glutils lib, not building effects")
#     endif (NOT KWIN_GLUTILS)
# 
#     if (NOT OPENGL)
#         message(STATUS "didnt find opengl, not building effects")
#     endif (NOT OPENGL)
# 
#     if (EFFECTS_H AND KWIN_GLUTILS AND KWIN_EFFECTS AND OPENGL)
#         add_subdirectory(effects)
#     endif (EFFECTS_H AND KWIN_GLUTILS AND KWIN_EFFECTS AND OPENGL)

    set(SRCS
        kwinclient2.cpp
        decobutton.cpp
        menubar.cpp
        xcbatoms.cpp)
    set(HDRS
        kwinclient2.h
        decobutton.h
        decoadaptor.h
        menubar.h
        xcbatoms.h)

    ### build library
    add_library(atmodecoration MODULE ${SRCS} ${HDRS})

    target_link_libraries(atmodecoration
        PUBLIC
            Qt5::Core
            Qt5::Gui
            nsesettings
            atmolib
        PRIVATE
            KDecoration2::KDecoration
            KF5::ConfigCore
            KF5::CoreAddons
            KF5::ConfigWidgets
            KF5::GuiAddons
            KF5::WindowSystem)
    if (QTDBUS_FOUND)
        target_link_libraries(atmodecoration PUBLIC Qt5::DBus)
    endif ()

    if (dbusmenu-qt5_FOUND)
        target_link_libraries(atmodecoration PUBLIC dbusmenu-qt5)
        add_definitions(-DHASDBUSMENU=1)
    endif ()

    if (XCB_FOUND)
        target_link_libraries(atmodecoration PUBLIC Qt5::X11Extras XCB::XCB)
    else ()
        if (X11_FOUND)
            target_link_libraries(atmodecoration PUBLIC Qt5::X11Extras ${X11_X11_LIB} ${X11_LIBRARIES})
        endif ()
    endif ()
    
    install(TARGETS atmodecoration DESTINATION ${MODULEPATH}/org.kde.kdecoration2)
