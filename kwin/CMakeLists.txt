message(STATUS "preparing kwin deco")

if (QT5BUILD)
    #NOTE: most of this copied from the default breeze deco and partly rewritten for dsp
    find_package(KF5 REQUIRED COMPONENTS CoreAddons ConfigWidgets WindowSystem GuiAddons I18n)
    find_package(Qt5 CONFIG REQUIRED COMPONENTS DBus)
    
    include(KDEInstallDirs)
    include(KDECMakeSettings)
    include(KDECompilerSettings)
    include(GenerateExportHeader)

    set(SRCS kwinclient2.cpp decobutton.cpp)
    set(HDRS kwinclient2.h decobutton.h decoadaptor.h)
    
#     qt5_wrap_cpp(MOC_ADAPTOR decoadaptor.h)
    ### build library
    add_library(dspdecoration MODULE ${SRCS} ${HDRS})

    target_link_libraries(dspdecoration
        PUBLIC
            Qt5::Core
            Qt5::Gui
            Qt5::DBus
            styleproject_qt5
        PRIVATE
            KDecoration2::KDecoration
            KF5::ConfigCore
            KF5::CoreAddons
            KF5::ConfigWidgets
            KF5::GuiAddons
            KF5::I18n
            KF5::WindowSystem)

    if (XCB_FOUND)
        target_link_libraries(dspdecoration
            PUBLIC
            Qt5::X11Extras
            XCB::XCB)
    endif (XCB_FOUND)
    
    if (X11_FOUND)
        target_link_libraries(dspdecoration PUBLIC ${X11_X11_LIB} ${X11_LIBRARIES})
    endif (X11_FOUND)

#    install(TARGETS dspdecoration DESTINATION ${PLUGIN_INSTALL_DIR}/org.kde.kdecoration2)
    install(TARGETS dspdecoration DESTINATION lib/qt/plugins/org.kde.kdecoration2)
else (QT5BUILD)
    find_file(KDECORATION_H kdecoration.h ${KDE4_INCLUDES})
    if (KDECORATION_H)
        project(kwin3_styleproject)
        set(KWIN_SRCS factory.cpp kwinclient.cpp sizegrip.cpp)
	message(STATUS "kwin deco header found: " ${KDECORATION_H})
        kde4_add_plugin(kwin3_styleproject ${KWIN_SRCS})
        target_link_libraries(kwin3_styleproject styleproject ${QT_LIBRARIES} ${KDE4_KDEUI_LIBS} kdecorations ${X11_LIBRARIES})
        install(TARGETS kwin3_styleproject DESTINATION ${PLUGIN_INSTALL_DIR})
        install(FILES styleproject.desktop DESTINATION ${DATA_INSTALL_DIR}/kwin)
    else (KDECORATION_H)
	message(WARNING "kwin deco header not found, not building deco even if you have kde4")
    endif (KDECORATION_H)
endif (QT5BUILD)
