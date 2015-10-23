set(SHAPECORNERS kwin4_effect_dsp_shapecorners)

message(STATUS "configuring dsp effect to shape corners")

project(${SHAPECORNERS})

add_library(${SHAPECORNERS} MODULE shapecorners.cpp)

target_link_libraries(${SHAPECORNERS}
    PUBLIC
        Qt5::Core
        Qt5::Gui
        ${KWIN_EFFECTS}
        ${KWIN_GLUTILS}
        epoxy
        GL
    PRIVATE
        KF5::ConfigCore
        KF5::CoreAddons
        KF5::ConfigWidgets
        KF5::GuiAddons
        KF5::WindowSystem
        )

if (XCB_FOUND)
    target_link_libraries(${SHAPECORNERS} PUBLIC Qt5::X11Extras XCB::XCB)
else (XCB_FOUND)
    if (X11_FOUND)
        target_link_libraries(${SHAPECORNERS} PUBLIC ${X11_X11_LIB} ${X11_LIBRARIES})
    endif (X11_FOUND)
endif (XCB_FOUND)

#kcoreaddons_desktop_to_json(${SHAPECORNERS} shapecorners.desktop)

install(TARGETS ${SHAPECORNERS} DESTINATION ${MODULEPATH}/kwin/effects/plugins)
execute_process(COMMAND kf5-config --install services OUTPUT_VARIABLE SERVICEPATH OUTPUT_STRIP_TRAILING_WHITESPACE)
#install(FILES shapecorners.desktop DESTINATION ${SERVICEPATH}/kwin)

execute_process(COMMAND kf5-config --install data OUTPUT_VARIABLE DATAPATH OUTPUT_STRIP_TRAILING_WHITESPACE)

install(FILES shapecorners.frag DESTINATION ${DATAPATH}/kwin/shaders/1.40)
