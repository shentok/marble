PROJECT( marble-mobile )

INCLUDE_DIRECTORIES(
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_CURRENT_BINARY_DIR}
 ${QT_INCLUDE_DIR}
)
INCLUDE(${QT_USE_FILE})

set( my_SRCS
  main.cpp
 )

set( QML_FILES
    main.qml
    MainPage.qml
    RoutingPage.qml
    MapSelectionDialog.qml
)
marble_add_project_resources(${QML_FILES})

qt4_add_resources(my_SRCS main.qrc)
qt4_automoc( ${my_SRCS} )
add_executable( marble-mobile ${my_SRCS} )
target_link_libraries( marble-mobile ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTMAIN_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY} )
install( TARGETS marble-mobile RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
