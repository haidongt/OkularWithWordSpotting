include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
)


########### next target ###############

set(okularGenerator_kimgio_PART_SRCS generator_kimgio.cpp )


kde4_add_plugin(okularGenerator_kimgio ${okularGenerator_kimgio_PART_SRCS})

target_link_libraries(okularGenerator_kimgio okularcore ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY} )

install(TARGETS okularGenerator_kimgio DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES libokularGenerator_kimgio.desktop okularKimgio.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( PROGRAMS okularApplication_kimgio.desktop active-documentviewer_kimgio.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )

#install( FILES gui.rc DESTINATION ${DATA_INSTALL_DIR}/okular_kimgio )
